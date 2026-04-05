OUTPUT_FORMAT("elf64-x86-64")
ENTRY(kernel_main)

PH_BASE = 8M;
VM_OFFSET = 0xFFFFFFFF80000000;

SECTIONS
{
    . = VM_OFFSET + PH_BASE;

    .text : AT(PH_BASE)
    {
        _code_start = .;
        *(.text .text.*)
        . = ALIGN(16);
        _code_end = .;
    }

    .rodata : AT(ADDR(.rodata) - VM_OFFSET)
    {
        _rodata_start = .;
        *(.rodata .rodata.*)
        . = ALIGN(16);
        _rodata_end = .;
    }

    .data : AT(ADDR(.data) - VM_OFFSET)
    {
        _data_start = .;
        *(.data .data.*)
        . = ALIGN(16);
        _data_end = .;
    }

    .bss : AT(ADDR(.bss) - VM_OFFSET)
    {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(16);
        _bss_end = .;
    }

    /DISCARD/ : { *(.comment) *(.eh_frame) }
}