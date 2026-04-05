OUTPUT_FORMAT("elf64-x86-64")
ENTRY(long_mode_entry)

PH_BASE = 2M;
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

    .paging (ALIGN(4096)) : AT(ADDR(.paging) - VM_OFFSET)
    {
        *(.paging)
    }

    .region_array (ALIGN(4096)) : AT(ADDR(.region_array) - VM_OFFSET)
    {
        *(.region_array) 
    }

    _supervisor_end_phys = . - VM_OFFSET;
    ASSERT(_supervisor_end_phys <= 8M, "Supervisor segment too large and overlaps Kernel!")

    /DISCARD/ : { *(.comment) *(.eh_frame) }
}