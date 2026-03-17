OUTPUT_FORMAT("elf64-x86-64")
ENTRY(kernel_main)

SECTIONS
{
    . = 8M;

    .text : AT(0)
    {
        _code_start = .;
        *(.text .text.*)
        . = ALIGN(16);
        _code_end = .;
    }

    .rodata : AT(LOADADDR(.text) + SIZEOF(.text))
    {
        _rodata_start = .;
        *(.rodata .rodata.*)
        . = ALIGN(16);
        _rodata_end = .;
    }

    .data : AT(LOADADDR(.rodata) + SIZEOF(.rodata))
    {
        _data_start = .;
        *(.data .data.*)
        . = ALIGN(16);
        _data_end = .;
    }

    .bss : AT(LOADADDR(.data) + SIZEOF(.data))
    {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(16);
        _bss_end = .;
    }

    /DISCARD/ : { *(.comment) *(.eh_frame) }
}