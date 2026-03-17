OUTPUT_FORMAT("elf32-i386")
ENTRY(start)

SECTIONS {
    . = 1M;
    .boot : {
        KEEP(*(.multiboot))
        *(.text)
        *(.rodata)
        *(.data)
    }
    . = 2M;
    .supervisor : {
        KEEP(*(.supervisor_bundle)) 
    }

    . = 8M;
    .kernel : {
        KEEP(*(.kernel_bundle))    
    }

    . = ALIGN(4096);
    .bss : {
        *(COMMON)
        *(.bss)
    }

    _end = .;
}