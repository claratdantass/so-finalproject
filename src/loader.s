; entry point name that the linker and GRUB jump to
global loader
global kernel_stack

extern kmain
extern kernel_physical_start
extern kernel_physical_end

; multiboot header, GRUB looks for this to know it's a valid kernel
MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x3                ; align modules + request mem info
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS)

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

; kernel bootstrap: set up the stack and jump to C
loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; point esp to the start of the
                                                ; stack (end of memory area)
    push kernel_physical_end                     ; arg4
    push kernel_physical_start                   ; arg3
    push ebx                                     ; arg2 (multiboot info ptr)
    push eax                                     ; arg1 (multiboot magic)
    call kmain                                  ; transfer control to C

.loop:
    jmp .loop                                   ; safety net: never return

; ---- stack (uninitialized, lives in .bss to keep the binary small) ----
KERNEL_STACK_SIZE equ 4096                      ; 4 KB stack

section .bss
align 4                                         ; align at 4 bytes
kernel_stack:
    resb KERNEL_STACK_SIZE                      ; reserve stack for the kernel
