#include "ghost_boot_info.h"
#include "ghost_pmm.h"
#include "ghost_version.h"

#define GHOST_DEBUGCON_PORT 0x402
#define GHOST_PAGE_SIZE 4096ULL

static VOID debugcon_write_byte(UINT8 value)
{
    UINT16 port;

    port = GHOST_DEBUGCON_PORT;
    __asm__ volatile("outb %0, %w1" : : "a"(value), "Nd"(port));
}

static VOID debugcon_write_ascii(const char *text)
{
    while (*text != '\0') {
        debugcon_write_byte((UINT8)*text);
        ++text;
    }
}

static VOID debugcon_write_decimal64(UINT64 value)
{
    char    digits[20]; /* UINT64 max = 18446744073709551615 (20 digits) */
    UINT32  count;
    UINT32  i;

    if (value == 0) {
        debugcon_write_byte('0');
        return;
    }

    count = 0;

    while (value != 0 && count < (UINT32)(sizeof(digits) / sizeof(digits[0]))) {
        digits[count] = (char)('0' + (UINT8)(value % 10ULL));
        value /= 10ULL;
        ++count;
    }

    i = count;

    while (i > 0) {
        --i;
        debugcon_write_byte((UINT8)digits[i]);
    }
}

static VOID debugcon_write_hex64(UINT64 value)
{
    char    digits[16];
    UINT32  count;
    UINT32  i;
    UINT8   nibble;

    debugcon_write_ascii("0x");

    if (value == 0) {
        debugcon_write_byte('0');
        return;
    }

    count = 0;

    while (value != 0 && count < (UINT32)(sizeof(digits) / sizeof(digits[0]))) {
        nibble     = (UINT8)(value & 0xFULL);
        digits[count] = (char)((nibble < 10U) ? ('0' + nibble) : ('A' + nibble - 10U));
        value >>= 4U;
        ++count;
    }

    i = count;

    while (i > 0) {
        --i;
        debugcon_write_byte((UINT8)digits[i]);
    }
}

static UINT8 is_page_aligned(UINT64 value)
{
    return (value & (GHOST_PAGE_SIZE - 1ULL)) == 0ULL;
}

static UINT8 boot_info_has_required_fields(const GhostBootInfo_t *boot_info)
{
    if (boot_info == (VOID *)0) {
        return 0;
    }

    if ((boot_info->flags & GHOST_BOOT_INFO_FLAG_KERNEL_PAYLOAD_VALID) == 0ULL ||
        (boot_info->flags & GHOST_BOOT_INFO_FLAG_MEMORY_MAP_VALID) == 0ULL ||
        (boot_info->flags & GHOST_BOOT_INFO_FLAG_KERNEL_STACK_VALID) == 0ULL) {
        return 0;
    }

    if (boot_info->kernel_payload.physical_address == 0 ||
        boot_info->kernel_payload.entry_point == 0 ||
        (boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_KERNEL_OWNED) == 0U ||
        (boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_PAGE_ALIGNED) == 0U ||
        !is_page_aligned(boot_info->kernel_payload.physical_address) ||
        boot_info->memory_map.physical_address == 0 ||
        boot_info->memory_map.descriptor_size == 0 ||
        (boot_info->memory_map.flags & GHOST_MEMORY_MAP_FLAG_KERNEL_OWNED) == 0U ||
        (boot_info->memory_map.flags & GHOST_MEMORY_MAP_FLAG_FINAL_FOR_EXIT) == 0U ||
        (boot_info->memory_map.flags & GHOST_MEMORY_MAP_FLAG_PAGE_ALIGNED) == 0U ||
        !is_page_aligned(boot_info->memory_map.physical_address) ||
        boot_info->kernel_stack.top_address == 0) {
        return 0;
    }

    if ((boot_info->kernel_stack.flags & GHOST_KERNEL_STACK_FLAG_KERNEL_OWNED) == 0U ||
        (boot_info->kernel_stack.flags & GHOST_KERNEL_STACK_FLAG_GROWS_DOWN) == 0U ||
        (boot_info->kernel_stack.flags & GHOST_KERNEL_STACK_FLAG_16_BYTE_ALIGNED) == 0U ||
        (boot_info->kernel_stack.flags & GHOST_KERNEL_STACK_FLAG_PAGE_ALIGNED) == 0U ||
        !is_page_aligned(boot_info->kernel_stack.physical_address)) {
        return 0;
    }

    if ((boot_info->flags & GHOST_BOOT_INFO_FLAG_FRAMEBUFFER_VALID) != 0ULL &&
        ((boot_info->framebuffer.flags & GHOST_FRAMEBUFFER_FLAG_MMIO_REGION) == 0U ||
         (boot_info->framebuffer.flags & GHOST_FRAMEBUFFER_FLAG_LOADER_BORROWED) == 0U)) {
        return 0;
    }

    return 1;
}

static VOID paint_framebuffer(const GhostBootInfo_t *boot_info)
{
    UINT32 color;
    UINT32 height;
    UINT32 pitch;
    volatile UINT32 *row;
    UINT32 width;
    UINT32 x;
    UINT32 y;

    if (boot_info == (VOID *)0 ||
        (boot_info->flags & GHOST_BOOT_INFO_FLAG_FRAMEBUFFER_VALID) == 0ULL ||
        boot_info->framebuffer.physical_address == 0 ||
        boot_info->framebuffer.pixels_per_scan_line == 0) {
        return;
    }

    width = boot_info->framebuffer.width;
    height = boot_info->framebuffer.height;
    pitch = boot_info->framebuffer.pixels_per_scan_line;

    if (width > 160U) {
        width = 160U;
    }

    if (height > 96U) {
        height = 96U;
    }

    color = 0x0018D8F0U;

    if (boot_info->framebuffer.pixel_format == PixelBlueGreenRedReserved8BitPerColor) {
        color = 0x00F0D818U;
    }

    row = (volatile UINT32 *)(UINTN)boot_info->framebuffer.physical_address;

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            row[x] = color;
        }

        row += pitch;
    }
}

static VOID halt_forever(VOID)
{
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

VOID kernel_stage0_main(const GhostBootInfo_t *boot_info)
{
    GhostPmmStats_t pmm_stats;
    UINT32          pmm_result;

    debugcon_write_ascii("ghOSt kernel v" GHOST_VERSION_STRING "\r\n");
    debugcon_write_ascii("kernel_stage0 entered\r\n");

    if (!boot_info_has_required_fields(boot_info)) {
        debugcon_write_ascii("kernel_stage0 invalid_boot_info\r\n");
        halt_forever();
    }

    debugcon_write_ascii("kernel_stage0 boot_info_ok\r\n");

    /* --------------------------------------------------------------------- *
     * Physical memory manager bring-up.
     * --------------------------------------------------------------------- */
    debugcon_write_ascii("kernel_stage0 pmm_init\r\n");
    pmm_result = ghost_pmm_init(&boot_info->memory_map);

    if (pmm_result != GHOST_PMM_SUCCESS) {
        debugcon_write_ascii("kernel_stage0 pmm_failed result=");
        debugcon_write_decimal64((UINT64)pmm_result);
        debugcon_write_ascii("\r\n");
        halt_forever();
    }

    ghost_pmm_get_stats(&pmm_stats);
    debugcon_write_ascii("kernel_stage0 pmm_ok pages_total=");
    debugcon_write_decimal64(pmm_stats.total_pages);
    debugcon_write_ascii(" pages_usable=");
    debugcon_write_decimal64(pmm_stats.usable_pages);
    debugcon_write_ascii(" pages_free=");
    debugcon_write_decimal64(pmm_stats.free_pages);
    debugcon_write_ascii(" bitmap_base=");
    debugcon_write_hex64(pmm_stats.bitmap_base_address);
    debugcon_write_ascii("\r\n");

    if ((boot_info->flags & GHOST_BOOT_INFO_FLAG_FRAMEBUFFER_VALID) != 0ULL) {
        paint_framebuffer(boot_info);
        debugcon_write_ascii("kernel_stage0 framebuffer_ok\r\n");
    }
    else {
        debugcon_write_ascii("kernel_stage0 headless_ok\r\n");
    }

    debugcon_write_ascii("kernel_stage0 halted\r\n");
    halt_forever();
}