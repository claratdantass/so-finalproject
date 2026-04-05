/* Ponto de entrada C: inicializa subsistemas, monta SOFS a partir do módulo
 * GRUB, cria o processo shell e inicia o escalonador. */

#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "fb.h"
#include "fs.h"
#include "kheap.h"
#include "kutil.h"
#include "multiboot.h"
#include "paging.h"
#include "pfa.h"
#include "pit.h"
#include "process.h"
#include "serial.h"

extern char kernel_stack[];
#define KERNEL_STACK_SIZE 4096U

struct fs_instance rootfs;

void kmain(unsigned int multiboot_magic,
           unsigned int multiboot_info_addr,
           unsigned int kernel_physical_start,
           unsigned int kernel_physical_end)
{
    multiboot_info_t *mbinfo = (multiboot_info_t *)multiboot_info_addr;
    multiboot_module_t *mod;
    int n;

    /* ---- CPU, segmentos, paginação identidade e interrupções ---- */
    tss_init((unsigned int)kernel_stack + KERNEL_STACK_SIZE, 0x10);
    gdt_init();
    tss_load();
    paging_enable_identity();

    serial_init(SERIAL_COM1_BASE);
    fb_clear();
    idt_init();

    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        fb_write("Invalid multiboot magic\n", 24);
        serial_write(SERIAL_COM1_BASE, "Invalid multiboot magic\n", 24);
        while (1)
            ;
    }

    /* ---- Memória física e heap do kernel ---- */
    pfa_init(multiboot_info_addr, kernel_physical_start, kernel_physical_end);
    kheap_init();

    if (!(mbinfo->flags & MULTIBOOT_INFO_MODS) || mbinfo->mods_count < 1) {
        fb_write("ERROR: No module from GRUB.\n", 28);
        goto halt;
    }

    mod = (multiboot_module_t *)mbinfo->mods_addr;
    pfa_reserve(mod->mod_start, mod->mod_end);

    if (fs_init(&rootfs, mod->mod_start) != 0) {
        fb_write("ERROR: Invalid filesystem.\n", 27);
        goto halt;
    }

    /* ---- SOFS no initrd e mensagem de boot ---- */
    fb_write("SOFS: ", 6);
    {
        n = fs_num_files(&rootfs);
        char digit = '0' + (char)n;
        fb_write(&digit, 1);
    }
    fb_write(" file(s) loaded.\n", 17);

    /* ---- Um processo inicial + timer preemptivo ---- */
    process_init();
    process_create(&rootfs, "shell");

    pit_init(50);

    fb_write("Starting shell...\n", 18);
    scheduler_start();

/* ---- Falha de boot ou caminho de erro: CPU em hlt ---- */
halt:
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
