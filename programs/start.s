extern main

section .text.start
global _start
_start:
    call main
    jmp $           ; safety: loop forever if main returns
