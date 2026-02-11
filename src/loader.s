; entry point name that the linker and GRUB jump to
global loader

; multiboot header, GRUB looks for this to know it's a valid kernel
MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x0
CHECKSUM     equ -MAGIC_NUMBER   ; so magic + flags + checksum = 0

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

; kernel entry: put a value in eax (we check it later to confirm we booted)
loader:
    mov eax, 0xCAFEBABE
.loop:
    jmp .loop
