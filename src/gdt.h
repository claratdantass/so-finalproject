/* GDT (Global Descriptor Table) -- segmentation setup for x86
 *
 * Segmentation lets the CPU access memory through segments. We use a minimal
 * GDT: null + code + data, both covering the full 4GB. Main goal: set up
 * privilege level 0 (kernel mode) for our segments.
 */

#ifndef GDT_H
#define GDT_H

/* load GDT and reload segment registers. must be called early from kmain */
void gdt_init(void);

#endif /* GDT_H */
