/* PIC (Programmable Interrupt Controller) - remapping and acknowledgment */

#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

/* Remap PIC IRQs to interrupts 32-47 to avoid CPU exception conflicts */
void pic_remap(void);

/* Acknowledge an interrupt so the PIC can send more */
void pic_acknowledge(unsigned int interrupt);

#endif /* INCLUDE_PIC_H */
