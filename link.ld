/* entry symbol, where execution starts */
ENTRY(loader)

SECTIONS {
    /* load kernel at 1MB */
    . = 0x00100000;
    kernel_virtual_start = .;
    kernel_physical_start = .;

    .text ALIGN(0x1000) : {
        *(.text)
    }

    .rodata ALIGN(0x1000) : {
        *(.rodata*)
    }

    .data ALIGN(0x1000) : {
        *(.data)
    }

    /* uninitialized data */
    .bss ALIGN(0x1000) : {
        *(COMMON)
        *(.bss)
    }

    kernel_virtual_end = .;
    kernel_physical_end = .;
}
