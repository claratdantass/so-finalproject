; IDT loader: wraps the lidt instruction for use from C

global load_idt

; load_idt - Loads the Interrupt Descriptor Table
; stack: [esp + 4] pointer to IDT descriptor (limit + base)
;        [esp    ] return address
load_idt:
    mov     eax, [esp + 4]      ; load address of IDT descriptor
    lidt    [eax]               ; load the IDT
    ret
