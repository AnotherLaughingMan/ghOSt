#pragma once

#include "ghost_uefi.h"

#define GHOST_KERNEL_IMAGE_MAGIC "GHOSTKERNEL-X64"
#define GHOST_KERNEL_IMAGE_FORMAT_REVISION 1U

#define GHOST_KERNEL_IMAGE_MACHINE_X64 0x8664U

#define GHOST_KERNEL_IMAGE_SUBSYSTEM_NATIVE 1U

#define GHOST_KERNEL_IMAGE_ENTRY_ABI_X64_BOOT_INFO 1U

typedef struct {
    UINT8 magic[16];
    UINT32 format_revision;
    UINT32 header_size;
    UINT64 image_size;
    UINT64 entry_offset;
    UINT32 machine;
    UINT32 subsystem;
    UINT32 entry_abi;
    UINT32 minimum_boot_info_revision;
    UINT32 flags;
    UINT32 reserved;
} GhostKernelImageHeader_t;