#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

/* gdt_init:
 * Sets up a flat GDT with kernel/user code and data segments.
 * This gives us privilege separation through segmentation and
 * prepares the kernel for paging in the next chapter.
 */
void gdt_init(void);

#endif /* INCLUDE_GDT_H */
