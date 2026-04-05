#include "paging4k.h"
#include "pfa.h"
#include "kutil.h"

unsigned int *paging_create_kernel_dir(void)
{
    unsigned int frame;
    unsigned int *dir;
    unsigned int i;

    frame = pfa_alloc();
    if (frame == 0U) {
        return (unsigned int *)0;
    }

    dir = (unsigned int *)frame;

    for (i = 0; i < 1024; i++) {
        dir[i] = (i << 22) | PDE_PRESENT | PDE_RW | PDE_PS;
    }

    return dir;
}

void paging_map_4k(unsigned int *dir, unsigned int virt,
                   unsigned int phys, unsigned int flags)
{
    unsigned int pde_idx = virt >> 22;
    unsigned int pte_idx = (virt >> 12) & 0x3FFU;
    unsigned int *pt;
    unsigned int pt_frame;
    unsigned int i;

    if (dir[pde_idx] & PDE_PS) {
        /*
         * Split a 4 MiB PSE page into a page table of 1024 x 4 KiB pages.
         * Each PTE identity-maps its portion of the original 4 MiB range
         * with supervisor-only permissions (matching the original PDE).
         */
        unsigned int base = dir[pde_idx] & 0xFFC00000U;

        pt_frame = pfa_alloc();
        if (pt_frame == 0U) {
            return;
        }
        pt = (unsigned int *)pt_frame;

        for (i = 0; i < 1024; i++) {
            pt[i] = (base + i * PAGE_SIZE) | PTE_PRESENT | PTE_RW;
        }

        dir[pde_idx] = pt_frame | PDE_PRESENT | PDE_RW;
        if (flags & PTE_USER) {
            dir[pde_idx] |= PDE_USER;
        }

    } else if (!(dir[pde_idx] & PDE_PRESENT)) {
        pt_frame = pfa_alloc();
        if (pt_frame == 0U) {
            return;
        }
        pt = (unsigned int *)pt_frame;
        memset(pt, 0, PAGE_SIZE);

        dir[pde_idx] = pt_frame | PDE_PRESENT | PDE_RW;
        if (flags & PTE_USER) {
            dir[pde_idx] |= PDE_USER;
        }

    } else {
        pt = (unsigned int *)(dir[pde_idx] & 0xFFFFF000U);

        if ((flags & PTE_USER) && !(dir[pde_idx] & PDE_USER)) {
            dir[pde_idx] |= PDE_USER;
        }
    }

    pt[pte_idx] = (phys & 0xFFFFF000U) | flags;
}

void paging_switch(unsigned int dir_phys)
{
    __asm__ __volatile__("mov %0, %%cr3" : : "r"(dir_phys) : "memory");
}
