; entry point name that the linker and GRUB jump to
global loader

; kmain is defined in kmain.c
extern kmain

; multiboot header, GRUB looks for this to know it's a valid kernel
MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x0
CHECKSUM     equ -MAGIC_NUMBER   ; so magic + flags + checksum = 0

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

; kernel bootstrap: set up the stack and jump to C
loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; point esp to the start of the
                                                ; stack (end of memory area)
    call kmain                                  ; transfer control to C

.loop:
    jmp .loop                                   ; safety net: never return

; ---- stack (uninitialized, lives in .bss to keep the binary small) ----
KERNEL_STACK_SIZE equ 4096                      ; 4 KB stack

section .bss
align 4                                         ; align at 4 bytes
kernel_stack:
    resb KERNEL_STACK_SIZE                      ; reserve stack for the kernel
