; Interrupt handler stubs and common dispatcher
; Uses NASM macros to generate a handler for each interrupt (0-255).
; Pushes a dummy error code (0) for interrupts that don't produce one.

; Macro for interrupts that do NOT push an error code
%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword 0                     ; push 0 as fake error code
    push    dword %1                    ; push interrupt number
    jmp     common_interrupt_handler
%endmacro

; Macro for interrupts that DO push an error code
%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword %1                    ; push interrupt number (error code already on stack)
    jmp     common_interrupt_handler
%endmacro

; C handler: void interrupt_handler(struct cpu_state cpu, unsigned int int_no,
;                                   struct stack_state stack)
extern interrupt_handler

; Common handler: save registers, call C handler, restore, return
common_interrupt_handler:
    ; save general-purpose registers
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp

    ; call C interrupt dispatcher
    call    interrupt_handler

    ; restore general-purpose registers
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    ; remove interrupt number and error code from stack
    add     esp, 8

    ; return from interrupt
    iret

; CPU exceptions (0-31)
no_error_code_interrupt_handler 0       ; Division by zero
no_error_code_interrupt_handler 1       ; Debug
no_error_code_interrupt_handler 2       ; NMI
no_error_code_interrupt_handler 3       ; Breakpoint
no_error_code_interrupt_handler 4       ; Overflow
no_error_code_interrupt_handler 5       ; Bound range exceeded
no_error_code_interrupt_handler 6       ; Invalid opcode
no_error_code_interrupt_handler 7       ; Device not available
error_code_interrupt_handler    8       ; Double fault
no_error_code_interrupt_handler 9       ; Coprocessor segment overrun
error_code_interrupt_handler    10      ; Invalid TSS
error_code_interrupt_handler    11      ; Segment not present
error_code_interrupt_handler    12      ; Stack-segment fault
error_code_interrupt_handler    13      ; General protection fault
error_code_interrupt_handler    14      ; Page fault
no_error_code_interrupt_handler 15      ; Reserved
no_error_code_interrupt_handler 16      ; x87 FPU error
error_code_interrupt_handler    17      ; Alignment check
no_error_code_interrupt_handler 18      ; Machine check
no_error_code_interrupt_handler 19      ; SIMD floating-point

; Interrupts 20-31: reserved by Intel
no_error_code_interrupt_handler 20
no_error_code_interrupt_handler 21
no_error_code_interrupt_handler 22
no_error_code_interrupt_handler 23
no_error_code_interrupt_handler 24
no_error_code_interrupt_handler 25
no_error_code_interrupt_handler 26
no_error_code_interrupt_handler 27
no_error_code_interrupt_handler 28
no_error_code_interrupt_handler 29
no_error_code_interrupt_handler 30
no_error_code_interrupt_handler 31

; IRQs 0-15 (remapped to interrupts 32-47 by the PIC)
no_error_code_interrupt_handler 32      ; IRQ 0 - Timer
no_error_code_interrupt_handler 33      ; IRQ 1 - Keyboard
no_error_code_interrupt_handler 34      ; IRQ 2 - Cascade (PIC2)
no_error_code_interrupt_handler 35      ; IRQ 3 - COM2
no_error_code_interrupt_handler 36      ; IRQ 4 - COM1
no_error_code_interrupt_handler 37      ; IRQ 5 - LPT2
no_error_code_interrupt_handler 38      ; IRQ 6 - Floppy disk
no_error_code_interrupt_handler 39      ; IRQ 7 - LPT1
no_error_code_interrupt_handler 40      ; IRQ 8 - Real time clock
no_error_code_interrupt_handler 41      ; IRQ 9 - General I/O
no_error_code_interrupt_handler 42      ; IRQ 10 - General I/O
no_error_code_interrupt_handler 43      ; IRQ 11 - General I/O
no_error_code_interrupt_handler 44      ; IRQ 12 - General I/O
no_error_code_interrupt_handler 45      ; IRQ 13 - Coprocessor
no_error_code_interrupt_handler 46      ; IRQ 14 - IDE bus
no_error_code_interrupt_handler 47      ; IRQ 15 - IDE bus
