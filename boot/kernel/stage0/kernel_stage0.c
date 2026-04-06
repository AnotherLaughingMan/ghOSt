#include "ghost_boot_info.h"

#define GHOST_DEBUGCON_PORT 0x402

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

    if (boot_info->kernel_payload.entry_point == 0 ||
        (boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_KERNEL_OWNED) == 0U ||
        boot_info->memory_map.physical_address == 0 ||
        boot_info->memory_map.descriptor_size == 0 ||
        (boot_info->memory_map.flags & GHOST_MEMORY_MAP_FLAG_KERNEL_OWNED) == 0U ||
        (boot_info->memory_map.flags & GHOST_MEMORY_MAP_FLAG_FINAL_FOR_EXIT) == 0U ||
        boot_info->kernel_stack.top_address == 0) {
        return 0;
    }

    if ((boot_info->kernel_stack.flags & GHOST_KERNEL_STACK_FLAG_KERNEL_OWNED) == 0U ||
        (boot_info->kernel_stack.flags & GHOST_KERNEL_STACK_FLAG_GROWS_DOWN) == 0U ||
        (boot_info->kernel_stack.flags & GHOST_KERNEL_STACK_FLAG_16_BYTE_ALIGNED) == 0U) {
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
    debugcon_write_ascii("kernel_stage0 entered\r\n");

    if (!boot_info_has_required_fields(boot_info)) {
        debugcon_write_ascii("kernel_stage0 invalid_boot_info\r\n");
        halt_forever();
    }

    debugcon_write_ascii("kernel_stage0 boot_info_ok\r\n");

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