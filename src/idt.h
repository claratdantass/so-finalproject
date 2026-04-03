/* IDT (Interrupt Descriptor Table) - interrupt/exception handling setup */

#ifndef IDT_H
#define IDT_H

/* Initialize the IDT with handlers for interrupts 0-47 and load it */
void idt_init(void);

#endif /* IDT_H */
