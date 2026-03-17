OUTPUT_FORMAT("elf64-x86-64")
ENTRY(long_mode_entry)

SECTIONS
{
    . = 2M;

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

    . = ALIGN(4096);
    .paging (NOLOAD) : 
    {
        *(.paging)
    }

    . = ALIGN(4096);
    .region_array (NOLOAD) : 
    {
        *(.region_array) 
    }

    _supervisor_end = .;
    ASSERT(. <= 8M, "Supervisor segment too large and overlaps Kernel!")

    /DISCARD/ : { *(.comment) *(.eh_frame) }
}