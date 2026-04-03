/* PIC driver: remaps hardware IRQs and handles acknowledgment
 *
 * By default the PIC maps IRQs 0-7 to interrupts 0-7, which conflicts
 * with CPU exceptions. We remap them to 32-47.
 */

#include "io.h"
#include "pic.h"

/* PIC I/O ports */
#define PIC1_PORT_A     0x20    /* PIC 1 command */
#define PIC1_PORT_B     0x21    /* PIC 1 data */
#define PIC2_PORT_A     0xA0    /* PIC 2 command */
#define PIC2_PORT_B     0xA1    /* PIC 2 data */

/* Remapped interrupt ranges */
#define PIC1_START_INTERRUPT 0x20   /* IRQ 0 -> interrupt 32 */
#define PIC2_START_INTERRUPT 0x28   /* IRQ 8 -> interrupt 40 */
#define PIC2_END_INTERRUPT   (PIC2_START_INTERRUPT + 7)

/* ICW (Initialization Command Word) values */
#define PIC_ICW1_INIT   0x11    /* init + ICW4 needed */
#define PIC_ICW4_8086   0x01    /* 8086/88 mode */
#define PIC_ACK         0x20    /* end-of-interrupt command */

/* pic_remap: remap PIC1 to 0x20-0x27 and PIC2 to 0x28-0x2F */
void pic_remap(void)
{
    /* ICW1: start initialization sequence on both PICs */
    outb(PIC1_PORT_A, PIC_ICW1_INIT);
    outb(PIC2_PORT_A, PIC_ICW1_INIT);

    /* ICW2: set interrupt vector offsets */
    outb(PIC1_PORT_B, PIC1_START_INTERRUPT);    /* PIC1 -> 0x20 */
    outb(PIC2_PORT_B, PIC2_START_INTERRUPT);    /* PIC2 -> 0x28 */

    /* ICW3: tell PIC1 there is a slave at IRQ2, tell PIC2 its cascade identity */
    outb(PIC1_PORT_B, 0x04);   /* IRQ2 has slave */
    outb(PIC2_PORT_B, 0x02);   /* slave identity */

    /* ICW4: set 8086 mode */
    outb(PIC1_PORT_B, PIC_ICW4_8086);
    outb(PIC2_PORT_B, PIC_ICW4_8086);

    /* Mask all IRQs except IRQ1 (keyboard) */
    outb(PIC1_PORT_B, 0xFD);   /* 1111 1101 -> only IRQ1 enabled */
    outb(PIC2_PORT_B, 0xFF);   /* all masked on PIC2 */
}

/* pic_acknowledge: send EOI to the appropriate PIC */
void pic_acknowledge(unsigned int interrupt)
{
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }

    /* If interrupt came from PIC2, acknowledge both PICs */
    if (interrupt >= PIC2_START_INTERRUPT) {
        outb(PIC2_PORT_A, PIC_ACK);
    }

    /* Always acknowledge PIC1 (master) */
    outb(PIC1_PORT_A, PIC_ACK);
}
