/* kmain -- kernel entry point in C
 *
 * Called by the assembly bootstrap (loader.s) after the stack is set up.
 * No standard library is available.
 */
#include "gdt.h"
#include "idt.h"
#include "fb.h"

void kmain(void)
{
    /* set up segmentation (GDT) before doing anything else */
    gdt_init();

    /* clear screen and show boot message */
    fb_clear();
    fb_write("Kernel loaded. Initializing interrupts...\n", 42);

    /* set up IDT, remap PIC, enable hardware interrupts */
    idt_init();

    fb_write("IDT loaded. Keyboard input enabled.\n", 36);

    /* halt until next interrupt, saving power */
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
