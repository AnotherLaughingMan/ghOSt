#pragma once

#include "ghost_uefi.h"

#define GHOST_BOOT_INFO_REVISION 1U

#define GHOST_KERNEL_PAYLOAD_FLAG_HEADER_VALID 0x00000001U
#define GHOST_KERNEL_PAYLOAD_FLAG_ENTRY_VALID  0x00000002U
#define GHOST_KERNEL_PAYLOAD_FLAG_ABI_VALID    0x00000004U
#define GHOST_KERNEL_PAYLOAD_FLAG_KERNEL_OWNED 0x00000008U

#define GHOST_FRAMEBUFFER_FLAG_MMIO_REGION     0x00000001U
#define GHOST_FRAMEBUFFER_FLAG_LOADER_BORROWED 0x00000002U

#define GHOST_MEMORY_MAP_FLAG_KERNEL_OWNED     0x00000001U
#define GHOST_MEMORY_MAP_FLAG_FINAL_FOR_EXIT   0x00000002U

#define GHOST_KERNEL_STACK_FLAG_KERNEL_OWNED   0x00000001U
#define GHOST_KERNEL_STACK_FLAG_GROWS_DOWN     0x00000002U
#define GHOST_KERNEL_STACK_FLAG_16_BYTE_ALIGNED 0x00000004U

#define GHOST_BOOT_INFO_FLAG_KERNEL_PAYLOAD_VALID 0x0000000000000001ULL
#define GHOST_BOOT_INFO_FLAG_FRAMEBUFFER_VALID   0x0000000000000002ULL
#define GHOST_BOOT_INFO_FLAG_MEMORY_MAP_VALID    0x0000000000000004ULL
#define GHOST_BOOT_INFO_FLAG_KERNEL_STACK_VALID  0x0000000000000008ULL

typedef struct {
    UINT64 physical_address;
    UINT64 size_bytes;
    UINT64 entry_point;
    UINT32 flags;
    UINT32 reserved;
} GhostKernelPayloadInfo_t;

typedef struct {
    UINT64 physical_address;
    UINT64 size_bytes;
    UINT32 width;
    UINT32 height;
    UINT32 pixels_per_scan_line;
    UINT32 pixel_format;
    UINT32 flags;
    UINT32 reserved;
} GhostFramebufferInfo_t;

typedef struct {
    UINT64 physical_address;
    UINT64 size_bytes;
    UINT64 map_key;
    UINT32 descriptor_size;
    UINT32 descriptor_version;
    UINT32 descriptor_count;
    UINT32 flags;
} GhostMemoryMapInfo_t;

typedef struct {
    UINT64 physical_address;
    UINT64 size_bytes;
    UINT64 top_address;
    UINT32 flags;
    UINT32 reserved;
} GhostKernelStackInfo_t;

typedef struct {
    UINT32 revision;
    UINT32 size;
    UINT64 flags;
    GhostKernelPayloadInfo_t kernel_payload;
    GhostFramebufferInfo_t framebuffer;
    GhostMemoryMapInfo_t memory_map;
    GhostKernelStackInfo_t kernel_stack;
} GhostBootInfo_t;