; loads GDT into CPU and reloads segment registers
; called from C with pointer to gdt_ptr struct (limit + base)

global gdt_load

gdt_load:
    mov eax, [esp + 4]       ; get pointer to gdt_ptr from stack
    lgdt [eax]               ; load GDT (limit and base) into GDTR

    ; reload data segment registers (selector 0x10 = index 2, GDT, RPL 0)
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; cs can only be changed by a far jump: set new cs (0x08) and jump
    jmp 0x08:.flush_cs
.flush_cs:
    ret
