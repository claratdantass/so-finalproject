#ifndef INCLUDE_TSS_H
#define INCLUDE_TSS_H

void tss_init(unsigned int kernel_stack_top, unsigned int kernel_data_segment);
void tss_load(void);
void tss_set_kernel_stack(unsigned int stack_top);
unsigned int tss_get_base(void);
unsigned int tss_get_limit(void);

#endif /* INCLUDE_TSS_H */
