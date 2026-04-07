#include "ghost_pmm.h"

/*
 * ghost_pmm.c — early physical memory manager.
 *
 * Bitmap allocator seeded from the final post-ExitBootServices UEFI memory
 * map.  One bit per 4 KiB page frame; bit set = free, bit clear = used.
 *
 * The bitmap is placed in the first EfiConventionalMemory region that is
 * large enough to hold it, and those pages are then marked used so they
 * cannot be reclaimed by the allocator.
 */

/* -------------------------------------------------------------------------
 * Module-level state — zero-initialised by the flat binary (.bss region).
 * ------------------------------------------------------------------------- */

static UINT8  *s_bitmap;           /* physical address of the bitmap buffer  */
static UINT64  s_bitmap_bit_count; /* total page-frame entries tracked       */
static UINT64  s_free_pages;       /* current count of free (set) bits       */
static UINT64  s_usable_pages;     /* pages in reclaimable regions at init   */
static UINT8   s_initialized;      /* non-zero after successful ghost_pmm_init */

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */

static UINT64 pmm_div_round_up(UINT64 numerator, UINT64 denominator)
{
    return (numerator + denominator - 1ULL) / denominator;
}

static void pmm_bitmap_set_bit(UINT64 bit_index)
{
    s_bitmap[bit_index >> 3U] |= (UINT8)(1U << (UINT8)(bit_index & 7ULL));
}

static void pmm_bitmap_clear_bit(UINT64 bit_index)
{
    s_bitmap[bit_index >> 3U] &= (UINT8)(~(1U << (UINT8)(bit_index & 7ULL)));
}

static UINT8 pmm_bitmap_test_bit(UINT64 bit_index)
{
    return (s_bitmap[bit_index >> 3U] >> (UINT8)(bit_index & 7ULL)) & 1U;
}

/* Zero the first byte_count bytes of the bitmap — marks all pages as used. */
static void pmm_clear_bitmap(UINT64 byte_count)
{
    UINT64 i;

    for (i = 0; i < byte_count; ++i) {
        s_bitmap[i] = 0;
    }
}

/*
 * Mark a range of page frames as free (set bits).
 * Clamps automatically to s_bitmap_bit_count.
 * Guards against double-marking with a test before increment.
 */
static void pmm_mark_range_free(UINT64 first_page, UINT64 page_count)
{
    UINT64 end_page;
    UINT64 i;

    end_page = first_page + page_count;

    if (end_page > s_bitmap_bit_count) {
        end_page = s_bitmap_bit_count;
    }

    for (i = first_page; i < end_page; ++i) {
        if (!pmm_bitmap_test_bit(i)) {
            pmm_bitmap_set_bit(i);
            ++s_free_pages;
        }
    }
}

/*
 * Mark a range of page frames as used (clear bits).
 * Clamps automatically to s_bitmap_bit_count.
 * Guards against double-marking with a test before decrement.
 */
static void pmm_mark_range_used(UINT64 first_page, UINT64 page_count)
{
    UINT64 end_page;
    UINT64 i;

    end_page = first_page + page_count;

    if (end_page > s_bitmap_bit_count) {
        end_page = s_bitmap_bit_count;
    }

    for (i = first_page; i < end_page; ++i) {
        if (pmm_bitmap_test_bit(i)) {
            pmm_bitmap_clear_bit(i);
            --s_free_pages;
        }
    }
}

/*
 * Returns 1 if the memory type is reclaimable as free after ExitBootServices.
 * EfiLoaderData (kernel payload, stack, map buffer) is intentionally excluded
 * since those pages are tracked via boot-info and managed by the kernel.
 */
static UINT8 pmm_is_free_region(UINT32 type)
{
    switch ((EFI_MEMORY_TYPE)type) {
    case EfiConventionalMemory:
    case EfiBootServicesCode:
    case EfiBootServicesData:
        return 1;
    default:
        return 0;
    }
}

/* -------------------------------------------------------------------------
 * Public interface
 * ------------------------------------------------------------------------- */

UINT32 ghost_pmm_init(const GhostMemoryMapInfo_t *memory_map)
{
    const UINT8              *descriptor_base;
    const EFI_MEMORY_DESCRIPTOR *descriptor;
    UINT64 entry_count;
    UINT64 highest_end_address;
    UINT64 region_end;
    UINT64 bitmap_byte_count;
    UINT64 bitmap_page_count;
    UINT64 bitmap_home_address;
    UINT64 first_page;
    UINT64 page_count;
    UINT64 i;
    UINT8  bitmap_placed;

    if (s_initialized) {
        return GHOST_PMM_SUCCESS;
    }

    if (memory_map == (VOID *)0 ||
        memory_map->physical_address == 0 ||
        memory_map->descriptor_size == 0 ||
        memory_map->descriptor_count == 0) {
        return GHOST_PMM_ERROR_INVALID;
    }

    descriptor_base = (const UINT8 *)(UINTN)memory_map->physical_address;
    entry_count     = (UINT64)memory_map->descriptor_count;

    /* --------------------------------------------------------------------- *
     * Pass 1: find the highest physical end address across all RAM regions.
     * Only types that represent actual RAM are considered.  Holes, MMIO
     * apertures, platform-reserved windows, and PAL code regions are
     * excluded so that a firmware-mapped window at a high physical address
     * does not force a multi-MiB bitmap for a machine with modest RAM.
     * --------------------------------------------------------------------- */
    highest_end_address = 0;

    for (i = 0; i < entry_count; ++i) {
        descriptor = (const EFI_MEMORY_DESCRIPTOR *)(
            descriptor_base + i * (UINT64)memory_map->descriptor_size);

        switch ((EFI_MEMORY_TYPE)descriptor->type) {
        case EfiConventionalMemory:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiACPIReclaimMemory:
        case EfiACPIMemoryNVS:
        case EfiUnusableMemory:
        case EfiPersistentMemory:
            region_end = descriptor->physical_start +
                         descriptor->number_of_pages * GHOST_PMM_PAGE_SIZE_BYTES;

            if (region_end > highest_end_address) {
                highest_end_address = region_end;
            }

            break;
        default:
            /* Reserved, MMIO, MMIOPort, PalCode — not RAM, skip. */
            break;
        }
    }

    if (highest_end_address == 0) {
        return GHOST_PMM_ERROR_INVALID;
    }

    s_bitmap_bit_count = highest_end_address / GHOST_PMM_PAGE_SIZE_BYTES;
    bitmap_byte_count  = pmm_div_round_up(s_bitmap_bit_count, 8ULL);
    bitmap_page_count  = pmm_div_round_up(bitmap_byte_count, GHOST_PMM_PAGE_SIZE_BYTES);

    /* --------------------------------------------------------------------- *
     * Pass 2: find the first EfiConventionalMemory region large enough to
     * hold the bitmap.  Boot-service regions are not used here because we
     * need the bitmap home to be unconditionally safe to write before Pass 3.
     * --------------------------------------------------------------------- */
    bitmap_home_address = 0;
    bitmap_placed       = 0;

    for (i = 0; i < entry_count; ++i) {
        descriptor = (const EFI_MEMORY_DESCRIPTOR *)(
            descriptor_base + i * (UINT64)memory_map->descriptor_size);

        if ((EFI_MEMORY_TYPE)descriptor->type != EfiConventionalMemory) {
            continue;
        }

        if (descriptor->number_of_pages < bitmap_page_count) {
            continue;
        }

        bitmap_home_address = descriptor->physical_start;
        bitmap_placed       = 1;
        break;
    }

    if (!bitmap_placed) {
        return GHOST_PMM_ERROR_NO_MEMORY;
    }

    /* Anchor the bitmap pointer and zero it (all pages initially marked used). */
    s_bitmap     = (UINT8 *)(UINTN)bitmap_home_address;
    s_free_pages = 0;
    pmm_clear_bitmap(bitmap_byte_count);

    /* --------------------------------------------------------------------- *
     * Pass 3: mark all reclaimable pages as free and accumulate usable count.
     * --------------------------------------------------------------------- */
    s_usable_pages = 0;

    for (i = 0; i < entry_count; ++i) {
        descriptor = (const EFI_MEMORY_DESCRIPTOR *)(
            descriptor_base + i * (UINT64)memory_map->descriptor_size);

        if (!pmm_is_free_region(descriptor->type)) {
            continue;
        }

        first_page      = descriptor->physical_start / GHOST_PMM_PAGE_SIZE_BYTES;
        page_count      = descriptor->number_of_pages;
        s_usable_pages += page_count;
        pmm_mark_range_free(first_page, page_count);
    }

    /* --------------------------------------------------------------------- *
     * Claim the bitmap pages so they are never handed out to callers.
     * --------------------------------------------------------------------- */
    pmm_mark_range_used(
        bitmap_home_address / GHOST_PMM_PAGE_SIZE_BYTES,
        bitmap_page_count);

    s_initialized = 1;
    return GHOST_PMM_SUCCESS;
}

UINT64 ghost_pmm_alloc_page(void)
{
    UINT64 byte_count;
    UINT64 page_index;
    UINT64 i;
    UINT8  byte_val;
    UINT8  bit;

    if (!s_initialized || s_free_pages == 0) {
        return 0;
    }

    byte_count = pmm_div_round_up(s_bitmap_bit_count, 8ULL);

    for (i = 0; i < byte_count; ++i) {
        byte_val = s_bitmap[i];

        if (byte_val == 0) {
            continue;
        }

        for (bit = 0; bit < 8U; ++bit) {
            if ((byte_val >> bit) & 1U) {
                page_index = (i * 8ULL) + (UINT64)bit;

                if (page_index < s_bitmap_bit_count) {
                    pmm_bitmap_clear_bit(page_index);
                    --s_free_pages;
                    return page_index * GHOST_PMM_PAGE_SIZE_BYTES;
                }
            }
        }
    }

    return 0;
}

void ghost_pmm_free_page(UINT64 physical_address)
{
    UINT64 page_index;

    if (!s_initialized || physical_address == 0) {
        return;
    }

    if ((physical_address & (GHOST_PMM_PAGE_SIZE_BYTES - 1ULL)) != 0ULL) {
        return;
    }

    page_index = physical_address / GHOST_PMM_PAGE_SIZE_BYTES;

    if (page_index >= s_bitmap_bit_count) {
        return;
    }

    if (!pmm_bitmap_test_bit(page_index)) {
        pmm_bitmap_set_bit(page_index);
        ++s_free_pages;
    }
}

void ghost_pmm_get_stats(GhostPmmStats_t *stats_out)
{
    UINT64 bitmap_byte_count;
    UINT64 bitmap_page_count;

    if (stats_out == (VOID *)0) {
        return;
    }

    bitmap_byte_count  = pmm_div_round_up(s_bitmap_bit_count, 8ULL);
    bitmap_page_count  = pmm_div_round_up(bitmap_byte_count, GHOST_PMM_PAGE_SIZE_BYTES);

    stats_out->total_pages         = s_bitmap_bit_count;
    stats_out->usable_pages        = s_usable_pages;
    stats_out->free_pages          = s_free_pages;
    stats_out->bitmap_base_address = (UINT64)(UINTN)s_bitmap;
    stats_out->bitmap_page_count   = bitmap_page_count;
}
