; Loads the GDT and updates segment registers.
; cdecl:
;   [esp + 4] -> pointer to packed struct gdt_ptr

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10            ; kernel data selector (GDT entry 2)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.reload_cs     ; far jump to reload CS (GDT entry 1)

.reload_cs:
    ret
