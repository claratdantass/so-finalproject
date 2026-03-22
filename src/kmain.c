/* kmain -- kernel entry point in C
 *
 * Called by the assembly bootstrap (loader.s) after the stack is set up.
 * loader.s pushes ebx (multiboot info pointer) as the first argument.
 * No standard library is available.
 */
#include "gdt.h"
#include "idt.h"
#include "fb.h"
#include "kheap.h"
#include "multiboot.h"
#include "paging.h"
#include "pfa.h"
#include "serial.h"

typedef void (*call_module_t)(void);

void kmain(unsigned int multiboot_magic,
           unsigned int multiboot_info_addr,
           unsigned int kernel_physical_start,
           unsigned int kernel_physical_end)
{
    void *heap_probe;
    multiboot_info_t *mbinfo = (multiboot_info_t *)multiboot_info_addr;

    gdt_init();
    paging_enable_identity();

    serial_init(SERIAL_COM1_BASE);
    fb_clear();
    fb_write("Kernel loaded. Initializing interrupts...\n", 42);

    idt_init();

    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        fb_write("Invalid multiboot magic\n", 24);
        serial_write(SERIAL_COM1_BASE, "Invalid multiboot magic\n", 24);
        while (1)
            ;
    }

    pfa_init(multiboot_info_addr, kernel_physical_start, kernel_physical_end);
    kheap_init();

    heap_probe = kmalloc(128);
    if (heap_probe != (void *)0) {
        kfree(heap_probe);
    }

    fb_write("GDT initialized (chapter 8)\n", 28);
    serial_write(SERIAL_COM1_BASE, "GDT initialized (chapter 8)\n", 28);
    fb_write("Paging enabled (chapter 9)\n", 27);
    serial_write(SERIAL_COM1_BASE, "Paging enabled (chapter 9)\n", 27);
    fb_write("Page frame allocator ready (chapter 10)\n", 40);
    serial_write(SERIAL_COM1_BASE, "Page frame allocator ready (chapter 10)\n", 40);
    fb_write("Kernel heap ready (chapter 10)\n", 31);
    serial_write(SERIAL_COM1_BASE, "Kernel heap ready (chapter 10)\n", 31);
    fb_write("Hello from kernel\n", 18);
    serial_write(SERIAL_COM1_BASE, "Hello from serial\n", 18);

    fb_write("IDT loaded. Keyboard input enabled.\n", 36);

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
