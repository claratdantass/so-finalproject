; x86 paging setup (32-bit):
; - identity maps the full 4 GiB linear space
; - uses 4 MiB pages (PSE)
; - enables paging by loading CR3 and setting CR0.PG

global paging_enable_identity
global paging_invalidate
global page_directory

section .data
align 4096
page_directory:
%assign i 0
%rep 1024
    ; PDE = physical 4 MiB frame base | P | RW | PS
    dd ((i << 22) | 0x00000083)
%assign i i + 1
%endrep

section .text

; void paging_enable_identity(void)
paging_enable_identity:
    mov eax, page_directory
    mov cr3, eax

    mov eax, cr4
    or  eax, 0x00000010      ; CR4.PSE = 1 (4 MiB pages)
    mov cr4, eax

    mov eax, cr0
    or  eax, 0x80000000      ; CR0.PG = 1
    mov cr0, eax
    ret

; void paging_invalidate(void *virtual_addr)
paging_invalidate:
    mov eax, [esp + 4]
    invlpg [eax]
    ret
