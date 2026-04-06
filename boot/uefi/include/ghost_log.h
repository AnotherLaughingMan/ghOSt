#pragma once

#include "ghost_uefi.h"

VOID ghost_debugcon_write_ascii(const char *text);
VOID ghost_output_ascii(EFI_SYSTEM_TABLE *system_table, const char *text);
VOID ghost_output_log_line(EFI_SYSTEM_TABLE *system_table, const char *text);

UINTN ghost_append_char(char *buffer, UINTN offset, UINTN capacity, char value);
UINTN ghost_append_ascii(char *buffer, UINTN offset, UINTN capacity, const char *text);
UINTN ghost_append_u64_decimal(char *buffer, UINTN offset, UINTN capacity, UINT64 value);
UINTN ghost_append_u64_hex(char *buffer, UINTN offset, UINTN capacity, UINT64 value);

UINT8 ghost_buffer_starts_with_ascii(const UINT8 *buffer, UINTN buffer_size, const char *prefix);
const char *ghost_memory_type_to_ascii(UINT32 memory_type);
const char *ghost_pixel_format_to_ascii(EFI_GRAPHICS_PIXEL_FORMAT pixel_format);