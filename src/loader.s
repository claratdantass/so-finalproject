; entry point name that the linker and GRUB jump to
global loader

; kmain is defined in kmain.c
extern kmain

; multiboot header, GRUB looks for this to know it's a valid kernel
MAGIC_NUMBER    equ 0x1BADB002
ALIGN_MODULES   equ 0x00000001      ; tell GRUB to align modules on page boundaries
CHECKSUM        equ -(MAGIC_NUMBER + ALIGN_MODULES)

section .text
align 4
    dd MAGIC_NUMBER
    dd ALIGN_MODULES
    dd CHECKSUM

; kernel bootstrap: set up the stack and jump to C
loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; point esp to the start of the
                                                ; stack (end of memory area)
    push ebx                                    ; ebx = multiboot info pointer from GRUB
    call kmain                                  ; transfer control to C

.loop:
    jmp .loop                                   ; safety net: never return

; ---- stack (uninitialized, lives in .bss to keep the binary small) ----
KERNEL_STACK_SIZE equ 4096                      ; 4 KB stack

section .bss
align 4                                         ; align at 4 bytes
kernel_stack:
    resb KERNEL_STACK_SIZE                      ; reserve stack for the kernel
