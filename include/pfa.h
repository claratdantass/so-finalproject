#ifndef INCLUDE_PFA_H
#define INCLUDE_PFA_H

#define PAGE_SIZE 4096U

void pfa_init(unsigned int multiboot_info_addr,
              unsigned int kernel_physical_start,
              unsigned int kernel_physical_end);

unsigned int pfa_alloc(void);
void pfa_free(unsigned int frame_addr);

unsigned int pfa_alloc_contiguous(unsigned int frame_count);
void pfa_free_contiguous(unsigned int frame_addr, unsigned int frame_count);

unsigned int pfa_total_frames(void);
unsigned int pfa_used_frames(void);
unsigned int pfa_free_frames(void);

#endif /* INCLUDE_PFA_H */
