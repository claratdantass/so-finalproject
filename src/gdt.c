/* GDT setup: descriptors and init
 *
 * We need 3 entries: null (required), kernel code (RX), kernel data (RW).
 * Both code and data span 0x00000000 - 0xFFFFFFFF so we effectively get
 * a flat layout; segmentation is used for privilege levels (DPL 0).
 */

#include "gdt.h"

/* pointer passed to lgdt: limit (size-1) and base address */
struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* implemented in gdt.s - loads GDT and reloads segment registers */
extern void gdt_load(struct gdt_ptr *ptr);

/* 3 descriptors * 8 bytes. layout: limit, base, access, flags. */
static unsigned long long gdt[3];

/* assemble the GDT and call assembly to load it */
void gdt_init(void)
{
    /* null descriptor (index 0) - required, never used */
    gdt[0] = 0;

    /* kernel code (index 1, selector 0x08): execute + read, DPL 0, full 4GB */
    gdt[1] = 0x00CF9A000000FFFFULL;

    /* kernel data (index 2, selector 0x10): read + write, DPL 0, full 4GB */
    gdt[2] = 0x00CF92000000FFFFULL;

    /* pass struct to assembly: limit = 3*8 - 1, base = &gdt */
    struct gdt_ptr ptr;
    ptr.limit = sizeof(gdt) - 1;
    ptr.base = (unsigned int)&gdt;

    gdt_load(&ptr);
}
