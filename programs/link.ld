OUTPUT_FORMAT("binary")

SECTIONS
{
    . = 0;

    .text ALIGN(4) :
    {
        *(.text.start)
        *(.text)
    }

    .data ALIGN(4) :
    {
        *(.data)
    }

    .rodata ALIGN(4) :
    {
        *(.rodata*)
    }

    .bss ALIGN(4) :
    {
        *(.bss)
        *(COMMON)
    }
}
