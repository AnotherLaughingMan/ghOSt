#pragma once

#include "ghost_boot_info.h"

/*
 * Early physical memory manager interface.
 *
 * All functions use the module-level static state established by ghost_pmm_init().
 * Call ghost_pmm_init() exactly once, immediately after ExitBootServices, before
 * attempting any page allocations.
 *
 * Tracking granularity: 4 KiB pages.
 *
 * Regions classified as free on init:
 *   EfiConventionalMemory        — was always available
 *   EfiBootServicesCode          — reclaimable after ExitBootServices
 *   EfiBootServicesData          — reclaimable after ExitBootServices
 *
 * Everything else (LoaderData, RuntimeServices, MMIO, Reserved, etc.) is
 * left marked used.  The bitmap itself is carved from the first large-enough
 * EfiConventionalMemory region and its pages are marked used so they are
 * excluded from the allocatable pool.
 */

#define GHOST_PMM_SUCCESS           0U
#define GHOST_PMM_ERROR_INVALID     1U
#define GHOST_PMM_ERROR_NO_MEMORY   2U

#define GHOST_PMM_PAGE_SIZE_BYTES   4096ULL

typedef struct {
    UINT64 total_pages;         /* Total page-frame entries tracked in bitmap */
    UINT64 usable_pages;        /* Pages in reclaimable memory map regions    */
    UINT64 free_pages;          /* Currently allocatable pages                */
    UINT64 bitmap_base_address; /* Physical address where the bitmap lives    */
    UINT64 bitmap_page_count;   /* Pages consumed by the bitmap itself        */
} GhostPmmStats_t;

/*
 * ghost_pmm_init — initialise the PMM from the final UEFI memory map.
 *
 * Returns GHOST_PMM_SUCCESS on success, or an error code if the memory map
 * is invalid or no suitable region for the bitmap can be found.
 * Calling this function a second time is a no-op that returns SUCCESS.
 */
UINT32 ghost_pmm_init(const GhostMemoryMapInfo_t *memory_map);

/*
 * ghost_pmm_alloc_page — allocate one free physical page.
 *
 * Returns the page-aligned physical address of the allocated page, or 0 if
 * the PMM is not initialised or no free pages remain.
 */
UINT64 ghost_pmm_alloc_page(void);

/*
 * ghost_pmm_free_page — return a previously allocated page to the free pool.
 *
 * Silently ignores null, misaligned, or out-of-range addresses, and
 * silently ignores double-frees (page already in the free pool).
 */
void ghost_pmm_free_page(UINT64 physical_address);

/*
 * ghost_pmm_get_stats — fill *stats_out with the current PMM counters.
 *
 * Silently ignores a null stats_out pointer.
 */
void ghost_pmm_get_stats(GhostPmmStats_t *stats_out);
