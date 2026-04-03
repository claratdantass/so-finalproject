; Tiny test program loaded as a GRUB module.
; Compiled as a flat binary (nasm -f bin).
; Sets eax to a known value so we can verify execution via GDB.

    mov eax, 0xDEADBEEF

    jmp $
