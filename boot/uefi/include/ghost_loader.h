#pragma once

#include "ghost_boot_info.h"

EFI_STATUS ghost_run_phase1_loader(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    GhostBootInfo_t **boot_info_out);