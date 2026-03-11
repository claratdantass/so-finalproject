/* Multiboot header -- structs describing the info GRUB passes to the kernel.
 *
 * Based on the GNU Multiboot Specification:
 *   https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 *
 * Only the fields needed for module loading (Chapter 7) are included.
 */

#ifndef INCLUDE_MULTIBOOT_H
#define INCLUDE_MULTIBOOT_H

/* Bit masks for the 'flags' field in multiboot_info_t */
#define MULTIBOOT_INFO_MEMORY   0x00000001  /* mem_lower / mem_upper valid */
#define MULTIBOOT_INFO_BOOTDEV  0x00000002  /* boot_device valid */
#define MULTIBOOT_INFO_CMDLINE  0x00000004  /* cmdline valid */
#define MULTIBOOT_INFO_MODS     0x00000008  /* mods_count / mods_addr valid */

/* Module descriptor: one entry per module loaded by GRUB */
typedef struct {
    unsigned int mod_start;     /* physical start address of module */
    unsigned int mod_end;       /* physical end address of module */
    unsigned int cmdline;       /* pointer to module command-line string */
    unsigned int reserved;      /* must be 0 */
} __attribute__((packed)) multiboot_module_t;

/* Top-level info struct passed by GRUB in register ebx */
typedef struct {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;    /* number of modules loaded */
    unsigned int mods_addr;     /* physical address of first multiboot_module_t */
    /* remaining fields omitted -- not needed yet */
} __attribute__((packed)) multiboot_info_t;

#endif /* INCLUDE_MULTIBOOT_H */
