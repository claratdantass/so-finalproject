; enter_usermode: switch page directory, set user segments, iret to ring 3
;
; void enter_usermode(unsigned int eip, unsigned int esp_user,
;                     unsigned int page_dir_phys)
;
; Segment selectors with RPL=3:
;   User code: GDT index 3 -> 0x18 | 0x03 = 0x1B
;   User data: GDT index 4 -> 0x20 | 0x03 = 0x23

global enter_usermode

enter_usermode:
    mov eax, [esp + 12]         ; page_dir_phys
    mov cr3, eax                ; switch to user page directory

    mov ax, 0x23                ; user data selector (0x20 | RPL 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, [esp + 4]          ; eip  (user entry point)
    mov ebx, [esp + 8]          ; esp_user (user stack top)

    push dword 0x23             ; ss:  user data selector
    push ebx                    ; esp: user stack pointer
    push dword 0x0002           ; eflags: IF=0 (interrupts disabled), bit 1 reserved
    push dword 0x1B             ; cs:  user code selector (0x18 | RPL 3)
    push eax                    ; eip: user entry point

    iret
