BITS 64
DEFAULT REL

%define GHOST_BOOT_INFO_FLAG_KERNEL_STACK_VALID 0x0000000000000008
%define GHOST_KERNEL_STACK_TOP_OFFSET 144

%define GHOST_KERNEL_IMAGE_FORMAT_REVISION 1
%define GHOST_KERNEL_IMAGE_MACHINE_X64 0x8664
%define GHOST_KERNEL_IMAGE_SUBSYSTEM_NATIVE 1
%define GHOST_KERNEL_IMAGE_ENTRY_ABI_X64_BOOT_INFO 1
%define GHOST_BOOT_INFO_REVISION 1
%define GHOST_KERNEL_PAYLOAD_FLAG_KERNEL_OWNED 0x00000008
%define GHOST_FRAMEBUFFER_FLAG_MMIO_REGION 0x00000001
%define GHOST_FRAMEBUFFER_FLAG_LOADER_BORROWED 0x00000002
%define GHOST_MEMORY_MAP_FLAG_KERNEL_OWNED 0x00000001
%define GHOST_MEMORY_MAP_FLAG_FINAL_FOR_EXIT 0x00000002
%define GHOST_KERNEL_STACK_FLAG_KERNEL_OWNED 0x00000001
%define GHOST_KERNEL_STACK_FLAG_GROWS_DOWN 0x00000002
%define GHOST_KERNEL_STACK_FLAG_16_BYTE_ALIGNED 0x00000004

section .ghost_header

align 16
extern ghost_kernel_image_end
global ghost_kernel_image_header

ghost_kernel_image_header:
    db 'GHOSTKERNEL-X64', 0
    dd GHOST_KERNEL_IMAGE_FORMAT_REVISION
    dd ghost_kernel_image_header_end - ghost_kernel_image_header
    dq ghost_kernel_image_end - ghost_kernel_image_header
    dq _start - ghost_kernel_image_header
    dd GHOST_KERNEL_IMAGE_MACHINE_X64
    dd GHOST_KERNEL_IMAGE_SUBSYSTEM_NATIVE
    dd GHOST_KERNEL_IMAGE_ENTRY_ABI_X64_BOOT_INFO
    dd GHOST_BOOT_INFO_REVISION
    dd 0
    dd 0
ghost_kernel_image_header_end:

section .text

global _start
extern kernel_stage0_main

_start:
    mov rbx, rcx

    test rbx, rbx
    jz halt_forever

    mov rax, [rbx + 8]
    test rax, GHOST_BOOT_INFO_FLAG_KERNEL_STACK_VALID
    jz halt_forever

    mov rsp, [rbx + GHOST_KERNEL_STACK_TOP_OFFSET]
    test rsp, rsp
    jz halt_forever

    and rsp, -16

    mov rdi, rbx
    call kernel_stage0_main

halt_forever:
.halt_loop:
    cli
    hlt
    jmp .halt_loop
