/* kmain -- kernel entry point in C
 *
 * Called by the assembly bootstrap (loader.s) after the stack is set up.
 * No standard library is available.
 */
void kmain(void)
{
    /* Nothing to do yet -- just keep the kernel alive */
    while (1)
        ;
}
