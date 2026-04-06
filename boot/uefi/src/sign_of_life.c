#include "ghost_loader.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
    GhostBootInfo_t *boot_info;
    EFI_STATUS status;

    boot_info = (VOID *)0;
    status = ghost_run_phase1_loader(image_handle, system_table, &boot_info);
    (VOID)boot_info;

    if (status != EFI_SUCCESS &&
        system_table != (VOID *)0 &&
        system_table->boot_services != (VOID *)0 &&
        system_table->boot_services->stall != (VOID *)0) {
        system_table->boot_services->stall(1000000);
    }

    if (system_table != (VOID *)0 &&
        system_table->runtime_services != (VOID *)0 &&
        system_table->runtime_services->reset_system != (VOID *)0) {
        system_table->runtime_services->reset_system(EfiResetShutdown, status, 0, (VOID *)0);
    }

    return status;
}