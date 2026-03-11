/* kmain -- kernel entry point in C
 *
 * Called by the assembly bootstrap (loader.s) after the stack is set up.
 * loader.s pushes ebx (multiboot info pointer) as the first argument.
 * No standard library is available.
 */
#include "gdt.h"
#include "idt.h"
#include "fb.h"
#include "multiboot.h"

typedef void (*call_module_t)(void);

void kmain(unsigned int ebx)
{
    gdt_init();

    fb_clear();
    fb_write("Kernel loaded. Initializing interrupts...\n", 42);

    idt_init();

    fb_write("IDT loaded. Keyboard input enabled.\n", 36);

    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    if (!(mbinfo->flags & MULTIBOOT_INFO_MODS)) {
        fb_write("ERROR: No module info from GRUB.\n", 33);
        goto halt;
    }

    if (mbinfo->mods_count != 1) {
        fb_write("ERROR: Expected exactly 1 module.\n", 34);
        goto halt;
    }

    multiboot_module_t *mod = (multiboot_module_t *) mbinfo->mods_addr;
    unsigned int addr = mod->mod_start;

    fb_write("Module found. Jumping to program...\n", 36);

    call_module_t start_program = (call_module_t) addr;
    start_program();

halt:
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
