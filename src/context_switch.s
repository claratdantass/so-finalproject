; context_switch.s — assembly routines for process scheduling
;
; switch_context: saves callee-saved regs on current stack, switches ESP/CR3,
;                 restores callee-saved regs from new stack, returns.
;
; jump_to_process: one-way switch for bootstrapping the first process.
;
; process_start_trampoline: entry point for newly created processes.
;                           Sets user data segments and irets to user mode.

global switch_context
global jump_to_process
global process_start_trampoline

; void switch_context(unsigned int *old_esp, unsigned int new_esp, unsigned int new_cr3)
;
; Stack after pushes (5 regs = 20 bytes):
;   [esp+0]  EFLAGS    [esp+4]  EDI
;   [esp+8]  ESI       [esp+12] EBX
;   [esp+16] EBP       [esp+20] return address
;   [esp+24] old_esp   [esp+28] new_esp   [esp+32] new_cr3
switch_context:
    push ebp
    push ebx
    push esi
    push edi
    pushfd

    mov eax, [esp + 24]         ; eax = old_esp ptr
    mov ecx, [esp + 28]         ; ecx = new_esp
    mov edx, [esp + 32]         ; edx = new_cr3

    mov [eax], esp              ; save current ESP
    mov cr3, edx                ; switch page directory
    mov esp, ecx                ; switch to new stack

    popfd
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; void jump_to_process(unsigned int saved_esp, unsigned int page_dir_phys)
; One-way: loads process state without saving current state.
jump_to_process:
    mov ecx, [esp + 4]          ; saved_esp
    mov edx, [esp + 8]          ; page_dir_phys
    mov cr3, edx
    mov esp, ecx
    popfd
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; Trampoline for newly created processes.
; When switch_context returns here, the stack has an iret frame:
;   [esp+0]  EIP    [esp+4]  CS    [esp+8]  EFLAGS
;   [esp+12] ESP_user   [esp+16] SS_user
process_start_trampoline:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    iret
