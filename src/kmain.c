/* kmain -- kernel entry point in C
 *
 * Called by the assembly bootstrap (loader.s) after the stack is set up.
 * No standard library is available.
 */
#include "fb.h"
#include "serial.h"

void kmain(void)
{
    fb_clear();
    serial_init(SERIAL_COM1_BASE);

    fb_write("Hello from kernel\n", 18);
    serial_write(SERIAL_COM1_BASE, "Hello from serial\n", 18);

    while (1)
        ;
}
