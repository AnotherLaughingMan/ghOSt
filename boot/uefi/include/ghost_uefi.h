#pragma once

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef unsigned long long UINTN;
typedef short INT16;
typedef long long INT64;
typedef UINT16 CHAR16;
typedef void VOID;
typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;

typedef VOID *EFI_HANDLE;
typedef UINTN EFI_STATUS;

#define EFI_SUCCESS 0
#define EFI_INVALID_PARAMETER ((EFI_STATUS)0x8000000000000002ULL)
#define EFI_NOT_FOUND ((EFI_STATUS)0x800000000000000EULL)
#define EFI_BUFFER_TOO_SMALL ((EFI_STATUS)0x8000000000000005ULL)
#define EFI_LOAD_ERROR ((EFI_STATUS)0x8000000000000001ULL)

#define EFI_FILE_MODE_READ 0x0000000000000001ULL

#if defined(__x86_64__) || defined(_M_X64)
#define EFIAPI __attribute__((ms_abi))
#else
#define EFIAPI
#endif

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
} EFI_RESET_TYPE;

typedef struct {
    UINT64 signature;
    UINT32 revision;
    UINT32 header_size;
    UINT32 crc32;
    UINT32 reserved;
} EFI_TABLE_HEADER;

typedef struct {
    UINT16 scan_code;
    CHAR16 unicode_char;
} EFI_INPUT_KEY;

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef struct EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef struct EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
typedef struct EFI_FILE_INFO EFI_FILE_INFO;
typedef struct EFI_LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE_PROTOCOL;
typedef struct EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;
typedef struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;
typedef struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;
typedef struct EFI_MEMORY_DESCRIPTOR EFI_MEMORY_DESCRIPTOR;
typedef struct EFI_RUNTIME_SERVICES EFI_RUNTIME_SERVICES;
typedef struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
typedef struct EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;

typedef struct {
    UINT32 data1;
    UINT16 data2;
    UINT16 data3;
    UINT8 data4[8];
} EFI_GUID;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    UINT32 red_mask;
    UINT32 green_mask;
    UINT32 blue_mask;
    UINT32 reserved_mask;
} EFI_PIXEL_BITMASK;

typedef struct {
    UINT16 year;
    UINT8 month;
    UINT8 day;
    UINT8 hour;
    UINT8 minute;
    UINT8 second;
    UINT8 pad1;
    UINT32 nanosecond;
    INT16 time_zone;
    UINT8 daylight;
    UINT8 pad2;
} EFI_TIME;

typedef EFI_STATUS(EFIAPI *EFI_TEXT_RESET)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, UINT8 extended_verification);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, const CHAR16 *string);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_TEST_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, const CHAR16 *string);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_QUERY_MODE)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, UINTN mode_number, UINTN *columns, UINTN *rows);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_SET_MODE)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, UINTN mode_number);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, UINTN attribute);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_CLEAR_SCREEN)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_SET_CURSOR_POSITION)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, UINTN column, UINTN row);
typedef EFI_STATUS(EFIAPI *EFI_TEXT_ENABLE_CURSOR)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this_protocol, UINT8 visible);
typedef EFI_STATUS(EFIAPI *EFI_IMAGE_EXIT)(EFI_HANDLE image_handle, EFI_STATUS exit_status, UINTN exit_data_size, CHAR16 *exit_data);
typedef EFI_STATUS(EFIAPI *EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE image_handle, UINTN map_key);
typedef EFI_STATUS(EFIAPI *EFI_GET_MEMORY_MAP)(UINTN *memory_map_size, EFI_MEMORY_DESCRIPTOR *memory_map, UINTN *map_key, UINTN *descriptor_size, UINT32 *descriptor_version);
typedef EFI_STATUS(EFIAPI *EFI_ALLOCATE_POOL)(EFI_MEMORY_TYPE pool_type, UINTN size, VOID **buffer);
typedef EFI_STATUS(EFIAPI *EFI_FILE_CLOSE)(EFI_FILE_PROTOCOL *this_file);
typedef EFI_STATUS(EFIAPI *EFI_FILE_GET_INFO)(EFI_FILE_PROTOCOL *this_file, EFI_GUID *information_type, UINTN *buffer_size, VOID *buffer);
typedef EFI_STATUS(EFIAPI *EFI_FILE_OPEN)(EFI_FILE_PROTOCOL *this_file, EFI_FILE_PROTOCOL **new_handle, const CHAR16 *file_name, UINT64 open_mode, UINT64 attributes);
typedef EFI_STATUS(EFIAPI *EFI_FILE_READ)(EFI_FILE_PROTOCOL *this_file, UINTN *buffer_size, VOID *buffer);
typedef EFI_STATUS(EFIAPI *EFI_FREE_POOL)(VOID *buffer);
typedef EFI_STATUS(EFIAPI *EFI_HANDLE_PROTOCOL)(EFI_HANDLE handle, EFI_GUID *protocol, VOID **interface_out);
typedef EFI_STATUS(EFIAPI *EFI_LOCATE_PROTOCOL)(EFI_GUID *protocol, VOID *registration, VOID **interface_out);
typedef EFI_STATUS(EFIAPI *EFI_OPEN_VOLUME)(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *this_protocol, EFI_FILE_PROTOCOL **root);
typedef EFI_STATUS(EFIAPI *EFI_STALL)(UINTN microseconds);
typedef VOID(EFIAPI *EFI_RESET_SYSTEM)(EFI_RESET_TYPE reset_type, EFI_STATUS reset_status, UINTN data_size, VOID *reset_data);

struct EFI_MEMORY_DESCRIPTOR {
    UINT32 type;
    UINT32 pad;
    EFI_PHYSICAL_ADDRESS physical_start;
    EFI_VIRTUAL_ADDRESS virtual_start;
    UINT64 number_of_pages;
    UINT64 attribute;
};

struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
    UINT32 version;
    UINT32 horizontal_resolution;
    UINT32 vertical_resolution;
    EFI_GRAPHICS_PIXEL_FORMAT pixel_format;
    EFI_PIXEL_BITMASK pixel_information;
    UINT32 pixels_per_scan_line;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
    UINT32 max_mode;
    UINT32 mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN size_of_info;
    EFI_PHYSICAL_ADDRESS frame_buffer_base;
    UINTN frame_buffer_size;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    VOID *query_mode;
    VOID *set_mode;
    VOID *blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *mode;
};

struct EFI_FILE_PROTOCOL {
    UINT64 revision;
    EFI_FILE_OPEN open;
    EFI_FILE_CLOSE close;
    VOID *delete_file;
    EFI_FILE_READ read;
    VOID *write;
    VOID *get_position;
    VOID *set_position;
    EFI_FILE_GET_INFO get_info;
    VOID *set_info;
    VOID *flush;
    VOID *open_ex;
    VOID *read_ex;
    VOID *write_ex;
    VOID *flush_ex;
};

struct EFI_FILE_INFO {
    UINT64 size;
    UINT64 file_size;
    UINT64 physical_size;
    EFI_TIME create_time;
    EFI_TIME last_access_time;
    EFI_TIME modification_time;
    UINT64 attribute;
    CHAR16 file_name[1];
};

struct EFI_LOADED_IMAGE_PROTOCOL {
    UINT32 revision;
    EFI_HANDLE parent_handle;
    EFI_SYSTEM_TABLE *system_table;
    EFI_HANDLE device_handle;
    VOID *file_path;
    VOID *reserved;
    UINT32 load_options_size;
    VOID *load_options;
    VOID *image_base;
    UINT64 image_size;
    EFI_MEMORY_TYPE image_code_type;
    EFI_MEMORY_TYPE image_data_type;
    VOID *unload;
};

struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64 revision;
    EFI_OPEN_VOLUME open_volume;
};

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET reset;
    EFI_TEXT_STRING output_string;
    EFI_TEXT_TEST_STRING test_string;
    EFI_TEXT_QUERY_MODE query_mode;
    EFI_TEXT_SET_MODE set_mode;
    EFI_TEXT_SET_ATTRIBUTE set_attribute;
    EFI_TEXT_CLEAR_SCREEN clear_screen;
    EFI_TEXT_SET_CURSOR_POSITION set_cursor_position;
    EFI_TEXT_ENABLE_CURSOR enable_cursor;
    VOID *mode;
};

struct EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER hdr;
    VOID *raise_tpl;
    VOID *restore_tpl;
    VOID *allocate_pages;
    VOID *free_pages;
    EFI_GET_MEMORY_MAP get_memory_map;
    EFI_ALLOCATE_POOL allocate_pool;
    EFI_FREE_POOL free_pool;
    VOID *create_event;
    VOID *set_timer;
    VOID *wait_for_event;
    VOID *signal_event;
    VOID *close_event;
    VOID *check_event;
    VOID *install_protocol_interface;
    VOID *reinstall_protocol_interface;
    VOID *uninstall_protocol_interface;
    EFI_HANDLE_PROTOCOL handle_protocol;
    VOID *reserved;
    VOID *register_protocol_notify;
    VOID *locate_handle;
    VOID *locate_device_path;
    VOID *install_configuration_table;
    VOID *load_image;
    VOID *start_image;
    EFI_IMAGE_EXIT exit;
    VOID *unload_image;
    EFI_EXIT_BOOT_SERVICES exit_boot_services;
    VOID *get_next_monotonic_count;
    EFI_STALL stall;
    VOID *set_watchdog_timer;
    VOID *connect_controller;
    VOID *disconnect_controller;
    VOID *open_protocol;
    VOID *close_protocol;
    VOID *open_protocol_information;
    VOID *protocols_per_handle;
    VOID *locate_handle_buffer;
    EFI_LOCATE_PROTOCOL locate_protocol;
    VOID *install_multiple_protocol_interfaces;
    VOID *uninstall_multiple_protocol_interfaces;
    VOID *calculate_crc32;
    VOID *copy_mem;
    VOID *set_mem;
    VOID *create_event_ex;
};

struct EFI_RUNTIME_SERVICES {
    EFI_TABLE_HEADER hdr;
    VOID *get_time;
    VOID *set_time;
    VOID *get_wakeup_time;
    VOID *set_wakeup_time;
    VOID *set_virtual_address_map;
    VOID *convert_pointer;
    VOID *get_variable;
    VOID *get_next_variable_name;
    VOID *set_variable;
    VOID *get_next_high_mono_count;
    EFI_RESET_SYSTEM reset_system;
    VOID *update_capsule;
    VOID *query_capsule_capabilities;
    VOID *query_variable_info;
};

struct EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER hdr;
    CHAR16 *firmware_vendor;
    UINT32 firmware_revision;
    EFI_HANDLE console_in_handle;
    VOID *con_in;
    EFI_HANDLE console_out_handle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *con_out;
    EFI_HANDLE standard_error_handle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *std_err;
    EFI_RUNTIME_SERVICES *runtime_services;
    EFI_BOOT_SERVICES *boot_services;
    UINTN number_of_table_entries;
    VOID *configuration_table;
};