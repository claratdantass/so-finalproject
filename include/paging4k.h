#ifndef INCLUDE_PAGING4K_H
#define INCLUDE_PAGING4K_H

#define PTE_PRESENT  0x01U
#define PTE_RW       0x02U
#define PTE_USER     0x04U

#define PDE_PRESENT  0x01U
#define PDE_RW       0x02U
#define PDE_USER     0x04U
#define PDE_PS       0x80U

unsigned int *paging_create_kernel_dir(void);

void paging_map_4k(unsigned int *dir, unsigned int virt,
                   unsigned int phys, unsigned int flags);

void paging_switch(unsigned int dir_phys);

#endif /* INCLUDE_PAGING4K_H */
