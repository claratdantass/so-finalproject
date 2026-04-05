#include "tss.h"
#include "kutil.h"

struct tss_entry {
    unsigned int prev_tss;
    unsigned int esp0;
    unsigned int ss0;
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short iomap_base;
} __attribute__((packed));

static struct tss_entry tss;

void tss_init(unsigned int kernel_stack_top, unsigned int kernel_data_segment)
{
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = kernel_data_segment;
    tss.esp0 = kernel_stack_top;
    tss.iomap_base = (unsigned short)sizeof(struct tss_entry);
}

void tss_load(void)
{
    __asm__ __volatile__(
        "movw $0x28, %%ax\n\t"
        "ltr %%ax"
        : : : "eax"
    );
}

void tss_set_kernel_stack(unsigned int stack_top)
{
    tss.esp0 = stack_top;
}

unsigned int tss_get_base(void)
{
    return (unsigned int)&tss;
}

unsigned int tss_get_limit(void)
{
    return (unsigned int)(sizeof(struct tss_entry) - 1U);
}
