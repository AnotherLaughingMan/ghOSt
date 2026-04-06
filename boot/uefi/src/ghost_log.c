#include "ghost_log.h"

#define GHOST_DEBUGCON_PORT 0x402
#define GHOST_TEXT_BUFFER_CAPACITY 128

static VOID ghost_debugcon_write_byte(UINT8 value)
{
    UINT16 port = GHOST_DEBUGCON_PORT;

    __asm__ volatile("outb %0, %w1" : : "a"(value), "Nd"(port));
}

VOID ghost_debugcon_write_ascii(const char *text)
{
    while (*text != '\0') {
        ghost_debugcon_write_byte((UINT8)*text);
        ++text;
    }
}

VOID ghost_output_ascii(EFI_SYSTEM_TABLE *system_table, const char *text)
{
    CHAR16 buffer[GHOST_TEXT_BUFFER_CAPACITY];
    UINTN length;

    if (system_table == (VOID *)0 || system_table->con_out == (VOID *)0) {
        return;
    }

    while (*text != '\0') {
        length = 0;

        while (*text != '\0' && length < (GHOST_TEXT_BUFFER_CAPACITY - 1)) {
            buffer[length] = (CHAR16)(UINT8)(*text);
            ++length;
            ++text;
        }

        buffer[length] = 0;
        system_table->con_out->output_string(system_table->con_out, buffer);
    }
}

VOID ghost_output_log_line(EFI_SYSTEM_TABLE *system_table, const char *text)
{
    ghost_debugcon_write_ascii(text);
    ghost_output_ascii(system_table, text);
}

UINTN ghost_append_char(char *buffer, UINTN offset, UINTN capacity, char value)
{
    if (offset + 1 < capacity) {
        buffer[offset] = value;
        ++offset;
        buffer[offset] = '\0';
    }

    return offset;
}

UINTN ghost_append_ascii(char *buffer, UINTN offset, UINTN capacity, const char *text)
{
    while (*text != '\0') {
        offset = ghost_append_char(buffer, offset, capacity, *text);
        ++text;
    }

    return offset;
}

UINTN ghost_append_u64_decimal(char *buffer, UINTN offset, UINTN capacity, UINT64 value)
{
    char digits[32];
    UINTN digit_count;

    if (value == 0) {
        return ghost_append_char(buffer, offset, capacity, '0');
    }

    digit_count = 0;

    while (value != 0 && digit_count < (sizeof(digits) / sizeof(digits[0]))) {
        digits[digit_count] = (char)('0' + (value % 10ULL));
        value /= 10ULL;
        ++digit_count;
    }

    while (digit_count > 0) {
        --digit_count;
        offset = ghost_append_char(buffer, offset, capacity, digits[digit_count]);
    }

    return offset;
}

UINTN ghost_append_u64_hex(char *buffer, UINTN offset, UINTN capacity, UINT64 value)
{
    char digits[16];
    UINTN digit_count;

    offset = ghost_append_ascii(buffer, offset, capacity, "0x");

    if (value == 0) {
        return ghost_append_char(buffer, offset, capacity, '0');
    }

    digit_count = 0;

    while (value != 0 && digit_count < (sizeof(digits) / sizeof(digits[0]))) {
        UINT8 nibble = (UINT8)(value & 0xFULL);

        if (nibble < 10U) {
            digits[digit_count] = (char)('0' + nibble);
        }
        else {
            digits[digit_count] = (char)('A' + (nibble - 10U));
        }

        value >>= 4U;
        ++digit_count;
    }

    while (digit_count > 0) {
        --digit_count;
        offset = ghost_append_char(buffer, offset, capacity, digits[digit_count]);
    }

    return offset;
}

UINT8 ghost_buffer_starts_with_ascii(const UINT8 *buffer, UINTN buffer_size, const char *prefix)
{
    UINTN index;

    index = 0;

    while (prefix[index] != '\0') {
        if (index >= buffer_size || buffer[index] != (UINT8)prefix[index]) {
            return 0;
        }

        ++index;
    }

    return 1;
}

const char *ghost_memory_type_to_ascii(UINT32 memory_type)
{
    switch ((EFI_MEMORY_TYPE)memory_type) {
    case EfiReservedMemoryType:
        return "Reserved";
    case EfiLoaderCode:
        return "LoaderCode";
    case EfiLoaderData:
        return "LoaderData";
    case EfiBootServicesCode:
        return "BootSvcCode";
    case EfiBootServicesData:
        return "BootSvcData";
    case EfiRuntimeServicesCode:
        return "RuntimeCode";
    case EfiRuntimeServicesData:
        return "RuntimeData";
    case EfiConventionalMemory:
        return "Conventional";
    case EfiUnusableMemory:
        return "Unusable";
    case EfiACPIReclaimMemory:
        return "ACPIReclaim";
    case EfiACPIMemoryNVS:
        return "ACPINVS";
    case EfiMemoryMappedIO:
        return "MMIO";
    case EfiMemoryMappedIOPortSpace:
        return "MMIOPort";
    case EfiPalCode:
        return "PalCode";
    case EfiPersistentMemory:
        return "Persistent";
    case EfiUnacceptedMemoryType:
        return "Unaccepted";
    default:
        return "Unknown";
    }
}

const char *ghost_pixel_format_to_ascii(EFI_GRAPHICS_PIXEL_FORMAT pixel_format)
{
    switch (pixel_format) {
    case PixelRedGreenBlueReserved8BitPerColor:
        return "RGBR8";
    case PixelBlueGreenRedReserved8BitPerColor:
        return "BGRR8";
    case PixelBitMask:
        return "BitMask";
    case PixelBltOnly:
        return "BltOnly";
    default:
        return "Unknown";
    }
}