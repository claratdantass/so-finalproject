#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "fb.h"
#include "kheap.h"
#include "kutil.h"
#include "multiboot.h"
#include "paging.h"
#include "paging4k.h"
#include "pfa.h"
#include "serial.h"
#include "usermode.h"

extern char kernel_stack[];
#define KERNEL_STACK_SIZE 4096U

#define USER_CODE_VIRT  0x00000000U
#define USER_STACK_VIRT 0xBFFFF000U
#define USER_STACK_TOP  0xC0000000U

void kmain(unsigned int multiboot_magic,
           unsigned int multiboot_info_addr,
           unsigned int kernel_physical_start,
           unsigned int kernel_physical_end)
{
    multiboot_info_t *mbinfo = (multiboot_info_t *)multiboot_info_addr;
    multiboot_module_t *mod;
    unsigned int mod_size;
    unsigned int code_pages;
    unsigned int *user_dir;
    unsigned int stack_frame;
    unsigned int code_frame;
    unsigned int offset;
    unsigned int copy_len;
    unsigned int i;

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

    pfa_init(multiboot_info_addr, kernel_physical_start, kernel_physical_end);
    kheap_init();

    if (!(mbinfo->flags & MULTIBOOT_INFO_MODS) || mbinfo->mods_count < 1) {
        fb_write("ERROR: No module from GRUB.\n", 28);
        goto halt;
    }

    mod = (multiboot_module_t *)mbinfo->mods_addr;
    mod_size = mod->mod_end - mod->mod_start;
    code_pages = (mod_size + PAGE_SIZE - 1U) / PAGE_SIZE;

    pfa_reserve(mod->mod_start, mod->mod_end);

    user_dir = paging_create_kernel_dir();
    if (user_dir == (unsigned int *)0) {
        fb_write("ERROR: Cannot alloc page dir.\n", 30);
        goto halt;
    }

    for (i = 0; i < code_pages; i++) {
        code_frame = pfa_alloc();
        if (code_frame == 0U) {
            fb_write("ERROR: Cannot alloc code frame.\n", 32);
            goto halt;
        }
        offset = i * PAGE_SIZE;
        copy_len = mod_size - offset;
        if (copy_len > PAGE_SIZE) {
            copy_len = PAGE_SIZE;
        }
        memcpy((void *)code_frame, (void *)(mod->mod_start + offset), copy_len);
        if (copy_len < PAGE_SIZE) {
            memset((void *)(code_frame + copy_len), 0, PAGE_SIZE - copy_len);
        }
        paging_map_4k(user_dir, USER_CODE_VIRT + offset, code_frame,
                       PTE_PRESENT | PTE_RW | PTE_USER);
    }

    stack_frame = pfa_alloc();
    if (stack_frame == 0U) {
        fb_write("ERROR: Cannot alloc stack.\n", 27);
        goto halt;
    }
    memset((void *)stack_frame, 0, PAGE_SIZE);
    paging_map_4k(user_dir, USER_STACK_VIRT, stack_frame,
                   PTE_PRESENT | PTE_RW | PTE_USER);

    fb_write("Entering user mode...\n", 22);

    enter_usermode(USER_CODE_VIRT, USER_STACK_TOP, (unsigned int)user_dir);

halt:
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
