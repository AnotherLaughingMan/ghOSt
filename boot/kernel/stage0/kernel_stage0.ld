ENTRY(_start)

SECTIONS
{
    . = 0;

    .ghost_header : ALIGN(16)
    {
        KEEP(*(.ghost_header))
    }

    .text : ALIGN(16)
    {
        *(.text*)
    }

    .rodata : ALIGN(16)
    {
        *(.rodata*)
    }

    .data : ALIGN(16)
    {
        *(.data*)
    }

    .bss : ALIGN(16)
    {
        *(.bss*)
        *(COMMON)
    }

    ghost_kernel_image_end = .;

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame*)
        *(.note*)
    }
}