#include "gdt.h"

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

extern void gdt_load(struct gdt_ptr *ptr);

static struct gdt_entry gdt[5];
static struct gdt_ptr gp;

static void gdt_set_gate(int idx, unsigned int base, unsigned int limit,
                         unsigned char access, unsigned char gran)
{
    gdt[idx].base_low = (unsigned short)(base & 0xFFFFU);
    gdt[idx].base_middle = (unsigned char)((base >> 16) & 0xFFU);
    gdt[idx].base_high = (unsigned char)((base >> 24) & 0xFFU);

    gdt[idx].limit_low = (unsigned short)(limit & 0xFFFFU);
    gdt[idx].granularity = (unsigned char)((limit >> 16) & 0x0FU);
    gdt[idx].granularity |= (unsigned char)(gran & 0xF0U);
    gdt[idx].access = access;
}

void gdt_init(void)
{
    gp.limit = (unsigned short)(sizeof(gdt) - 1U);
    gp.base = (unsigned int)&gdt;

    /* Null descriptor (required). */
    gdt_set_gate(0, 0U, 0U, 0U, 0U);

    /* Ring 0 code/data segments: base=0, limit=4 GiB (flat model). */
    gdt_set_gate(1, 0U, 0xFFFFFFFFU, 0x9AU, 0xCFU); /* code */
    gdt_set_gate(2, 0U, 0xFFFFFFFFU, 0x92U, 0xCFU); /* data */

    /* Ring 3 code/data segments for user-space isolation by privilege. */
    gdt_set_gate(3, 0U, 0xFFFFFFFFU, 0xFAU, 0xCFU); /* user code */
    gdt_set_gate(4, 0U, 0xFFFFFFFFU, 0xF2U, 0xCFU); /* user data */

    gdt_load(&gp);
}
