/* kmain -- kernel entry point in C
 *
 * Called by the assembly bootstrap (loader.s) after the stack is set up.
 * No standard library is available.
 */
#include "gdt.h"

void kmain(void)
{
    /* set up segmentation (GDT) before doing anything else */
    gdt_init();

    /* nothing else yet -- just keep the kernel alive */
    while (1)
        ;
}
