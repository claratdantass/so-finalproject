#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

/* Enables x86 paging using an identity-mapped page directory (4 MiB pages). */
void paging_enable_identity(void);

/* Invalidates one TLB entry for the given virtual address. */
void paging_invalidate(void *virtual_addr);

#endif /* INCLUDE_PAGING_H */
