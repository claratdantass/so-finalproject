; syscall_handler.s — int 0x80 entry point
;
; When int 0x80 fires from ring 3:
;   1. CPU switches to kernel stack (SS0:ESP0 from TSS)
;   2. CPU pushes: SS_user, ESP_user, EFLAGS, CS, EIP
;   3. We save GP registers, call C dispatcher with pointer to saved state
;   4. C dispatcher reads EAX (syscall number), may modify EAX (return value)
;   5. We restore GP registers and IRET back to ring 3

global syscall_handler_128
extern syscall_dispatch

syscall_handler_128:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp

    push esp                    ; arg: pointer to saved registers
    call syscall_dispatch
    add esp, 4

    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    iret
