#include "ghost_loader.h"
#include "ghost_kernel_image.h"
#include "ghost_log.h"

#define GHOST_KERNEL_STACK_ALIGNMENT 16ULL
#define GHOST_KERNEL_STACK_SIZE_BYTES (64ULL * 1024ULL)
#define GHOST_MEMORY_MAP_BUFFER_CAPACITY 32768
#define GHOST_PAGE_SIZE 4096ULL

static EFI_GUID g_loaded_image_protocol_guid = {
    0x5B1B31A1,
    0x9562,
    0x11D2,
    {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}
};

static EFI_GUID g_file_info_guid = {
    0x09576E92,
    0x6D3F,
    0x11D2,
    {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}
};

static EFI_GUID g_graphics_output_protocol_guid = {
    0x9042A9DE,
    0x23DC,
    0x4A38,
    {0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A}
};

static EFI_GUID g_simple_file_system_protocol_guid = {
    0x964E5B22,
    0x6459,
    0x11D2,
    {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}
};

static GhostBootInfo_t s_boot_info;
static UINT8 s_memory_map_buffer[GHOST_MEMORY_MAP_BUFFER_CAPACITY];

typedef VOID(EFIAPI *GhostKernelEntryPoint)(const GhostBootInfo_t *boot_info);

static UINT8 memory_type_is_usable_after_exit(UINT32 memory_type)
{
    switch ((EFI_MEMORY_TYPE)memory_type) {
    case EfiLoaderCode:
    case EfiLoaderData:
    case EfiBootServicesCode:
    case EfiBootServicesData:
    case EfiConventionalMemory:
        return 1;
    default:
        return 0;
    }
}

static VOID draw_gop_test_pattern(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop)
{
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINT32 *framebuffer;
    UINT32 width;
    UINT32 height;
    UINT32 pitch;
    UINT32 x;
    UINT32 y;
    UINT32 color_a;
    UINT32 color_b;

    if (gop == (VOID *)0 || gop->mode == (VOID *)0 || gop->mode->info == (VOID *)0) {
        return;
    }

    info = gop->mode->info;

    if (info->pixel_format != PixelRedGreenBlueReserved8BitPerColor &&
        info->pixel_format != PixelBlueGreenRedReserved8BitPerColor) {
        return;
    }

    if (gop->mode->frame_buffer_base == 0 || gop->mode->frame_buffer_size < sizeof(UINT32)) {
        return;
    }

    framebuffer = (UINT32 *)(UINTN)gop->mode->frame_buffer_base;
    pitch = info->pixels_per_scan_line;
    width = info->horizontal_resolution;
    height = info->vertical_resolution;

    if (width > 96U) {
        width = 96U;
    }

    if (height > 32U) {
        height = 32U;
    }

    if (info->pixel_format == PixelRedGreenBlueReserved8BitPerColor) {
        color_a = 0x0030D0F0U;
        color_b = 0x00F0D030U;
    }
    else {
        color_a = 0x00F0D030U;
        color_b = 0x0030D0F0U;
    }

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            UINT32 color = (((x / 16U) + (y / 16U)) % 2U == 0U) ? color_a : color_b;
            framebuffer[(UINTN)y * (UINTN)pitch + (UINTN)x] = color;
        }
    }
}

static VOID initialize_boot_info(VOID)
{
    UINTN index;
    UINT8 *boot_info_bytes;

    boot_info_bytes = (UINT8 *)&s_boot_info;

    for (index = 0; index < sizeof(s_boot_info); ++index) {
        boot_info_bytes[index] = 0;
    }

    s_boot_info.revision = GHOST_BOOT_INFO_REVISION;
    s_boot_info.size = (UINT32)sizeof(s_boot_info);
}

static VOID log_kernel_payload_status(EFI_SYSTEM_TABLE *system_table, const GhostBootInfo_t *boot_info)
{
    char line_buffer[192];
    UINTN line_length;

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload loaded size=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_payload.size_bytes);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " bytes\r\n");
    ghost_output_log_line(system_table, line_buffer);

    if ((boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_ENTRY_VALID) != 0U) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload entry_point=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_payload.entry_point);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
    }

    if ((boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_HEADER_VALID) != 0U) {
        ghost_output_log_line(system_table, "kernel_payload header_ok\r\n");
    }
    else {
        ghost_output_log_line(system_table, "kernel_payload header_mismatch\r\n");
    }

    if ((boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_ABI_VALID) != 0U) {
        ghost_output_log_line(system_table, "kernel_payload abi_ok\r\n");
    }

    if ((boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_KERNEL_OWNED) != 0U) {
        ghost_output_log_line(system_table, "kernel_payload ownership=kernel\r\n");
    }
}

static EFI_STATUS resolve_kernel_payload_entry(EFI_SYSTEM_TABLE *system_table, GhostBootInfo_t *boot_info)
{
    char line_buffer[192];
    const GhostKernelImageHeader_t *header;
    UINT64 entry_point;
    UINTN line_length;

    if (boot_info == (VOID *)0 ||
        boot_info->kernel_payload.physical_address == 0 ||
        boot_info->kernel_payload.size_bytes < sizeof(GhostKernelImageHeader_t)) {
        return EFI_LOAD_ERROR;
    }

    header = (const GhostKernelImageHeader_t *)(UINTN)boot_info->kernel_payload.physical_address;

    if (!ghost_buffer_starts_with_ascii((const UINT8 *)header, (UINTN)boot_info->kernel_payload.size_bytes, GHOST_KERNEL_IMAGE_MAGIC)) {
        return EFI_LOAD_ERROR;
    }

    if (header->format_revision != GHOST_KERNEL_IMAGE_FORMAT_REVISION ||
        header->machine != GHOST_KERNEL_IMAGE_MACHINE_X64 ||
        header->subsystem != GHOST_KERNEL_IMAGE_SUBSYSTEM_NATIVE ||
        header->entry_abi != GHOST_KERNEL_IMAGE_ENTRY_ABI_X64_BOOT_INFO ||
        header->minimum_boot_info_revision > GHOST_BOOT_INFO_REVISION ||
        header->header_size < sizeof(GhostKernelImageHeader_t) ||
        header->header_size > boot_info->kernel_payload.size_bytes ||
        header->image_size == 0 ||
        header->image_size > boot_info->kernel_payload.size_bytes ||
        header->entry_offset < header->header_size ||
        header->entry_offset >= header->image_size) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload header_invalid format_revision=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)header->format_revision);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " machine=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)header->machine);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " subsystem=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)header->subsystem);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " entry_abi=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)header->entry_abi);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " min_boot_info_revision=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)header->minimum_boot_info_revision);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " header_size=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)header->header_size);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " entry_offset=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), header->entry_offset);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " image_size=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), header->image_size);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return EFI_LOAD_ERROR;
    }

    entry_point = boot_info->kernel_payload.physical_address + header->entry_offset;
    boot_info->kernel_payload.entry_point = entry_point;
    boot_info->kernel_payload.flags |=
        GHOST_KERNEL_PAYLOAD_FLAG_HEADER_VALID |
        GHOST_KERNEL_PAYLOAD_FLAG_ENTRY_VALID |
        GHOST_KERNEL_PAYLOAD_FLAG_ABI_VALID |
        GHOST_KERNEL_PAYLOAD_FLAG_KERNEL_OWNED;
    return EFI_SUCCESS;
}

static EFI_STATUS load_kernel_payload(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    GhostBootInfo_t *boot_info)
{
    char line_buffer[192];
    const CHAR16 *kernel_payload_path = (const CHAR16 *)L"\\ghOSt\\kernel.bin";
    EFI_FILE_INFO *file_info;
    EFI_FILE_PROTOCOL *file;
    EFI_FILE_PROTOCOL *root;
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *simple_fs;
    EFI_STATUS status;
    UINT8 *payload_buffer;
    UINTN bytes_to_read;
    UINTN file_info_size;
    UINTN line_length;

    file_info = (VOID *)0;
    file = (VOID *)0;
    root = (VOID *)0;
    loaded_image = (VOID *)0;
    simple_fs = (VOID *)0;
    payload_buffer = (VOID *)0;

    if (system_table == (VOID *)0 ||
        system_table->boot_services == (VOID *)0 ||
        system_table->boot_services->handle_protocol == (VOID *)0 ||
        system_table->boot_services->allocate_pool == (VOID *)0 ||
        system_table->boot_services->free_pool == (VOID *)0) {
        return EFI_LOAD_ERROR;
    }

    status = system_table->boot_services->handle_protocol(
        image_handle,
        &g_loaded_image_protocol_guid,
        (VOID **)&loaded_image);

    if (status != EFI_SUCCESS || loaded_image == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload loaded_image_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    status = system_table->boot_services->handle_protocol(
        loaded_image->device_handle,
        &g_simple_file_system_protocol_guid,
        (VOID **)&simple_fs);

    if (status != EFI_SUCCESS || simple_fs == (VOID *)0 || simple_fs->open_volume == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload simple_fs_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    status = simple_fs->open_volume(simple_fs, &root);

    if (status != EFI_SUCCESS || root == (VOID *)0 || root->open == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload open_volume_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        goto cleanup;
    }

    status = root->open(root, &file, kernel_payload_path, EFI_FILE_MODE_READ, 0);

    if (status != EFI_SUCCESS || file == (VOID *)0 || file->get_info == (VOID *)0 || file->read == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload open_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        goto cleanup;
    }

    file_info_size = 0;
    status = file->get_info(file, &g_file_info_guid, &file_info_size, (VOID *)0);

    if (status != EFI_BUFFER_TOO_SMALL || file_info_size < sizeof(EFI_FILE_INFO)) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload get_info_size_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " header_size=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)file_info_size);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        goto cleanup;
    }

    status = system_table->boot_services->allocate_pool(EfiLoaderData, file_info_size, (VOID **)&file_info);

    if (status != EFI_SUCCESS || file_info == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload file_info_alloc_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        goto cleanup;
    }

    status = file->get_info(file, &g_file_info_guid, &file_info_size, file_info);

    if (status != EFI_SUCCESS || file_info->file_size == 0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload get_info_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " file_size=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), file_info->file_size);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        goto cleanup;
    }

    status = system_table->boot_services->allocate_pool(
        EfiLoaderData,
        (UINTN)file_info->file_size,
        (VOID **)&payload_buffer);

    if (status != EFI_SUCCESS || payload_buffer == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload buffer_alloc_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " size=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), file_info->file_size);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        goto cleanup;
    }

    bytes_to_read = (UINTN)file_info->file_size;
    status = file->read(file, &bytes_to_read, payload_buffer);

    if (status != EFI_SUCCESS || bytes_to_read != (UINTN)file_info->file_size) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_payload read_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " bytes=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)bytes_to_read);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " expected=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), file_info->file_size);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        goto cleanup;
    }

    boot_info->kernel_payload.physical_address = (UINT64)(UINTN)payload_buffer;
    boot_info->kernel_payload.size_bytes = file_info->file_size;
    boot_info->flags |= GHOST_BOOT_INFO_FLAG_KERNEL_PAYLOAD_VALID;

    status = resolve_kernel_payload_entry(system_table, boot_info);

    if (status != EFI_SUCCESS) {
        goto cleanup;
    }

    log_kernel_payload_status(system_table, boot_info);

cleanup:
    if (file_info != (VOID *)0) {
        system_table->boot_services->free_pool(file_info);
    }

    if (file != (VOID *)0 && file->close != (VOID *)0) {
        file->close(file);
    }

    if (root != (VOID *)0 && root->close != (VOID *)0) {
        root->close(root);
    }

    return status;
}

static EFI_STATUS capture_framebuffer(EFI_SYSTEM_TABLE *system_table, GhostBootInfo_t *boot_info)
{
    char line_buffer[192];
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status;
    UINTN line_length;

    if (system_table == (VOID *)0 ||
        system_table->boot_services == (VOID *)0 ||
        system_table->boot_services->locate_protocol == (VOID *)0) {
        return EFI_NOT_FOUND;
    }

    gop = (VOID *)0;
    status = system_table->boot_services->locate_protocol(
        &g_graphics_output_protocol_guid,
        (VOID *)0,
        (VOID **)&gop);

    if (status == EFI_NOT_FOUND) {
        ghost_output_log_line(system_table, "gop not_found\r\n");
        return status;
    }

    if (status != EFI_SUCCESS || gop == (VOID *)0 || gop->mode == (VOID *)0 || gop->mode->info == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "gop locate_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    boot_info->framebuffer.physical_address = gop->mode->frame_buffer_base;
    boot_info->framebuffer.size_bytes = (UINT64)gop->mode->frame_buffer_size;
    boot_info->framebuffer.width = gop->mode->info->horizontal_resolution;
    boot_info->framebuffer.height = gop->mode->info->vertical_resolution;
    boot_info->framebuffer.pixels_per_scan_line = gop->mode->info->pixels_per_scan_line;
    boot_info->framebuffer.pixel_format = (UINT32)gop->mode->info->pixel_format;
    boot_info->framebuffer.flags = GHOST_FRAMEBUFFER_FLAG_MMIO_REGION | GHOST_FRAMEBUFFER_FLAG_LOADER_BORROWED;
    boot_info->flags |= GHOST_BOOT_INFO_FLAG_FRAMEBUFFER_VALID;

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "gop mode=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)gop->mode->mode);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " resolution=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->framebuffer.width);
    line_length = ghost_append_char(line_buffer, line_length, sizeof(line_buffer), 'x');
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->framebuffer.height);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " format=");
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), ghost_pixel_format_to_ascii(gop->mode->info->pixel_format));
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " ppsl=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->framebuffer.pixels_per_scan_line);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
    ghost_output_log_line(system_table, line_buffer);

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "gop framebuffer_base=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->framebuffer.physical_address);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " framebuffer_size=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->framebuffer.size_bytes);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
    ghost_output_log_line(system_table, line_buffer);

    draw_gop_test_pattern(gop);
    return EFI_SUCCESS;
}

static EFI_STATUS capture_memory_map(EFI_SYSTEM_TABLE *system_table, GhostBootInfo_t *boot_info)
{
    char line_buffer[160];
    EFI_MEMORY_DESCRIPTOR *descriptor;
    EFI_GET_MEMORY_MAP get_memory_map;
    UINTN map_size;
    UINTN map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;
    UINTN descriptor_count;
    UINTN offset;
    UINT64 usable_bytes;
    EFI_STATUS status;
    UINTN line_length;

    if (system_table == (VOID *)0 ||
        system_table->boot_services == (VOID *)0 ||
        system_table->boot_services->get_memory_map == (VOID *)0) {
        return EFI_LOAD_ERROR;
    }

    get_memory_map = system_table->boot_services->get_memory_map;
    map_size = sizeof(s_memory_map_buffer);
    map_key = 0;
    descriptor_size = 0;
    descriptor_version = 0;

    status = get_memory_map(
        &map_size,
        (EFI_MEMORY_DESCRIPTOR *)s_memory_map_buffer,
        &map_key,
        &descriptor_size,
        &descriptor_version);

    if (status == EFI_BUFFER_TOO_SMALL) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "memory_map buffer_too_small required=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)map_size);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " bytes\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    if (status != EFI_SUCCESS || descriptor_size == 0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "memory_map get_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    descriptor_count = map_size / descriptor_size;
    usable_bytes = 0;

    boot_info->memory_map.physical_address = (UINT64)(UINTN)s_memory_map_buffer;
    boot_info->memory_map.size_bytes = (UINT64)map_size;
    boot_info->memory_map.map_key = (UINT64)map_key;
    boot_info->memory_map.descriptor_size = (UINT32)descriptor_size;
    boot_info->memory_map.descriptor_version = descriptor_version;
    boot_info->memory_map.descriptor_count = (UINT32)descriptor_count;
    boot_info->memory_map.flags = GHOST_MEMORY_MAP_FLAG_KERNEL_OWNED;
    boot_info->flags |= GHOST_BOOT_INFO_FLAG_MEMORY_MAP_VALID;

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "memory_map descriptors=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)descriptor_count);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " descriptor_size=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)descriptor_size);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " version=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)descriptor_version);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
    ghost_output_log_line(system_table, line_buffer);

    for (offset = 0; offset + descriptor_size <= map_size; offset += descriptor_size) {
        UINT64 region_bytes;

        descriptor = (EFI_MEMORY_DESCRIPTOR *)(s_memory_map_buffer + offset);

        if (!memory_type_is_usable_after_exit(descriptor->type)) {
            continue;
        }

        region_bytes = descriptor->number_of_pages * GHOST_PAGE_SIZE;
        usable_bytes += region_bytes;

        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "usable type=");
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), ghost_memory_type_to_ascii(descriptor->type));
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " start=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), descriptor->physical_start);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " pages=");
        line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), descriptor->number_of_pages);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " bytes=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), region_bytes);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
    }

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "usable_after_exit_total=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), usable_bytes);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " bytes\r\n");
    ghost_output_log_line(system_table, line_buffer);
    return EFI_SUCCESS;
}

static EFI_STATUS prepare_kernel_stack(EFI_SYSTEM_TABLE *system_table, GhostBootInfo_t *boot_info)
{
    char line_buffer[192];
    UINT64 raw_top_address;
    VOID *stack_buffer;
    EFI_STATUS status;
    UINTN line_length;

    if (system_table == (VOID *)0 ||
        system_table->boot_services == (VOID *)0 ||
        system_table->boot_services->allocate_pool == (VOID *)0) {
        return EFI_LOAD_ERROR;
    }

    stack_buffer = (VOID *)0;
    status = system_table->boot_services->allocate_pool(
        EfiLoaderData,
        (UINTN)GHOST_KERNEL_STACK_SIZE_BYTES,
        &stack_buffer);

    if (status != EFI_SUCCESS || stack_buffer == (VOID *)0) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_stack alloc_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " size=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), GHOST_KERNEL_STACK_SIZE_BYTES);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    raw_top_address = (UINT64)(UINTN)stack_buffer + GHOST_KERNEL_STACK_SIZE_BYTES;

    boot_info->kernel_stack.physical_address = (UINT64)(UINTN)stack_buffer;
    boot_info->kernel_stack.size_bytes = GHOST_KERNEL_STACK_SIZE_BYTES;
    boot_info->kernel_stack.top_address = raw_top_address & ~(GHOST_KERNEL_STACK_ALIGNMENT - 1ULL);
    boot_info->kernel_stack.flags =
        GHOST_KERNEL_STACK_FLAG_KERNEL_OWNED |
        GHOST_KERNEL_STACK_FLAG_GROWS_DOWN |
        GHOST_KERNEL_STACK_FLAG_16_BYTE_ALIGNED;
    boot_info->flags |= GHOST_BOOT_INFO_FLAG_KERNEL_STACK_VALID;

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "kernel_stack base=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_stack.physical_address);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " size=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_stack.size_bytes);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " top=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_stack.top_address);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
    ghost_output_log_line(system_table, line_buffer);
    return EFI_SUCCESS;
}

static EFI_STATUS refresh_memory_map_for_exit(EFI_SYSTEM_TABLE *system_table, GhostBootInfo_t *boot_info)
{
    UINTN descriptor_size;
    UINTN map_key;
    UINTN map_size;
    UINT32 descriptor_version;
    EFI_STATUS status;

    if (system_table == (VOID *)0 ||
        system_table->boot_services == (VOID *)0 ||
        system_table->boot_services->get_memory_map == (VOID *)0) {
        return EFI_LOAD_ERROR;
    }

    map_size = sizeof(s_memory_map_buffer);
    map_key = 0;
    descriptor_size = 0;
    descriptor_version = 0;

    status = system_table->boot_services->get_memory_map(
        &map_size,
        (EFI_MEMORY_DESCRIPTOR *)s_memory_map_buffer,
        &map_key,
        &descriptor_size,
        &descriptor_version);

    if (status != EFI_SUCCESS) {
        return status;
    }

    boot_info->memory_map.physical_address = (UINT64)(UINTN)s_memory_map_buffer;
    boot_info->memory_map.size_bytes = (UINT64)map_size;
    boot_info->memory_map.map_key = (UINT64)map_key;
    boot_info->memory_map.descriptor_size = (UINT32)descriptor_size;
    boot_info->memory_map.descriptor_version = descriptor_version;
    boot_info->memory_map.descriptor_count = (UINT32)(map_size / descriptor_size);
    boot_info->memory_map.flags = GHOST_MEMORY_MAP_FLAG_KERNEL_OWNED | GHOST_MEMORY_MAP_FLAG_FINAL_FOR_EXIT;
    boot_info->flags |= GHOST_BOOT_INFO_FLAG_MEMORY_MAP_VALID;

    return EFI_SUCCESS;
}

static EFI_STATUS exit_boot_services(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table, GhostBootInfo_t *boot_info)
{
    char line_buffer[192];
    EFI_STATUS status;
    UINTN line_length;

    if (system_table == (VOID *)0 ||
        system_table->boot_services == (VOID *)0 ||
        system_table->boot_services->get_memory_map == (VOID *)0 ||
        system_table->boot_services->exit_boot_services == (VOID *)0) {
        return EFI_LOAD_ERROR;
    }

    ghost_output_log_line(system_table, "exit_boot_services preparing\r\n");

    status = refresh_memory_map_for_exit(system_table, boot_info);

    if (status != EFI_SUCCESS) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "exit_boot_services map_refresh_failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "exit_boot_services map_ready key=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->memory_map.map_key);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " flags=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->memory_map.flags);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
    ghost_debugcon_write_ascii(line_buffer);

    status = system_table->boot_services->exit_boot_services(image_handle, (UINTN)boot_info->memory_map.map_key);

    if (status == EFI_INVALID_PARAMETER) {
        ghost_output_log_line(system_table, "exit_boot_services retry\r\n");

        status = refresh_memory_map_for_exit(system_table, boot_info);

        if (status != EFI_SUCCESS) {
            line_buffer[0] = '\0';
            line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "exit_boot_services retry_map_failed status=");
            line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
            line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
            ghost_output_log_line(system_table, line_buffer);
            return status;
        }

        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "exit_boot_services map_ready key=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->memory_map.map_key);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " flags=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->memory_map.flags);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_debugcon_write_ascii(line_buffer);

        status = system_table->boot_services->exit_boot_services(image_handle, (UINTN)boot_info->memory_map.map_key);
    }

    if (status != EFI_SUCCESS) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "exit_boot_services failed status=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)status);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
        return status;
    }

    ghost_debugcon_write_ascii("exit_boot_services ok\r\n");
    return EFI_SUCCESS;
}

static VOID EFIAPI jump_to_kernel_entry(EFI_SYSTEM_TABLE *system_table, const GhostBootInfo_t *boot_info)
{
    GhostKernelEntryPoint entry_point;

    if (boot_info == (VOID *)0 ||
        (boot_info->kernel_payload.flags & GHOST_KERNEL_PAYLOAD_FLAG_ENTRY_VALID) == 0U ||
        boot_info->kernel_payload.entry_point == 0) {
        ghost_debugcon_write_ascii("kernel_entry unavailable\r\n");
        if (system_table != (VOID *)0 && system_table->runtime_services != (VOID *)0 && system_table->runtime_services->reset_system != (VOID *)0) {
            system_table->runtime_services->reset_system(EfiResetShutdown, EFI_LOAD_ERROR, 0, (VOID *)0);
        }

        for (;;) {
        }
    }

    ghost_debugcon_write_ascii("kernel_entry jumping\r\n");
    entry_point = (GhostKernelEntryPoint)(UINTN)boot_info->kernel_payload.entry_point;
    entry_point(boot_info);
    ghost_debugcon_write_ascii("kernel_entry returned\r\n");

    if (system_table != (VOID *)0 && system_table->runtime_services != (VOID *)0 && system_table->runtime_services->reset_system != (VOID *)0) {
        system_table->runtime_services->reset_system(EfiResetShutdown, EFI_LOAD_ERROR, 0, (VOID *)0);
    }

    for (;;) {
    }
}

static VOID log_boot_info(EFI_SYSTEM_TABLE *system_table, const GhostBootInfo_t *boot_info)
{
    char line_buffer[192];
    UINTN line_length;

    line_buffer[0] = '\0';
    line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "boot_info revision=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->revision);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " size=");
    line_length = ghost_append_u64_decimal(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->size);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " flags=");
    line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->flags);
    line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
    ghost_output_log_line(system_table, line_buffer);

    if ((boot_info->flags & GHOST_BOOT_INFO_FLAG_KERNEL_PAYLOAD_VALID) != 0ULL) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "boot_info kernel_base=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_payload.physical_address);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " kernel_size=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_payload.size_bytes);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " kernel_flags=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->kernel_payload.flags);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
    }

    if ((boot_info->flags & GHOST_BOOT_INFO_FLAG_FRAMEBUFFER_VALID) != 0ULL) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "boot_info framebuffer_base=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->framebuffer.physical_address);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " framebuffer_size=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->framebuffer.size_bytes);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " framebuffer_flags=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->framebuffer.flags);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
    }

    if ((boot_info->flags & GHOST_BOOT_INFO_FLAG_MEMORY_MAP_VALID) != 0ULL) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "boot_info memory_map_base=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->memory_map.physical_address);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " memory_map_size=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->memory_map.size_bytes);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " map_key=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->memory_map.map_key);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " memory_map_flags=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->memory_map.flags);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
    }

    if ((boot_info->flags & GHOST_BOOT_INFO_FLAG_KERNEL_STACK_VALID) != 0ULL) {
        line_buffer[0] = '\0';
        line_length = ghost_append_ascii(line_buffer, 0, sizeof(line_buffer), "boot_info kernel_stack_base=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_stack.physical_address);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " kernel_stack_size=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_stack.size_bytes);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " kernel_stack_top=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), boot_info->kernel_stack.top_address);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), " kernel_stack_flags=");
        line_length = ghost_append_u64_hex(line_buffer, line_length, sizeof(line_buffer), (UINT64)boot_info->kernel_stack.flags);
        line_length = ghost_append_ascii(line_buffer, line_length, sizeof(line_buffer), "\r\n");
        ghost_output_log_line(system_table, line_buffer);
    }
}

EFI_STATUS ghost_run_phase1_loader(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    GhostBootInfo_t **boot_info_out)
{
    EFI_STATUS status;
    const char *banner = "ghOSt Phase 1 UEFI sign-of-life\r\n";
    const char *detail = "toolchain=clang+lld-link target=x86_64-pc-win32-coff\r\n";

    initialize_boot_info();

    ghost_debugcon_write_ascii(banner);
    ghost_debugcon_write_ascii(detail);
    ghost_output_ascii(system_table, banner);
    ghost_output_ascii(system_table, detail);

    status = load_kernel_payload(image_handle, system_table, &s_boot_info);

    if (status != EFI_SUCCESS) {
        return status;
    }

    capture_framebuffer(system_table, &s_boot_info);
    status = capture_memory_map(system_table, &s_boot_info);

    if (status != EFI_SUCCESS) {
        return status;
    }

    status = prepare_kernel_stack(system_table, &s_boot_info);

    if (status != EFI_SUCCESS) {
        return status;
    }

    log_boot_info(system_table, &s_boot_info);

    status = exit_boot_services(image_handle, system_table, &s_boot_info);

    if (status != EFI_SUCCESS) {
        return status;
    }

    if (boot_info_out != (VOID *)0) {
        *boot_info_out = &s_boot_info;
    }

    jump_to_kernel_entry(system_table, &s_boot_info);
    return EFI_LOAD_ERROR;
}