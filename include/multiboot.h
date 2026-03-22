#ifndef INCLUDE_MULTIBOOT_H
#define INCLUDE_MULTIBOOT_H

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/* multiboot_info flags */
#define MULTIBOOT_INFO_MEMORY  (1U << 0)
#define MULTIBOOT_INFO_BOOTDEV (1U << 1)
#define MULTIBOOT_INFO_CMDLINE (1U << 2)
#define MULTIBOOT_INFO_MODS    (1U << 3)
#define MULTIBOOT_INFO_MMAP    (1U << 6)

struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
} __attribute__((packed));

struct multiboot_mmap_entry {
    unsigned int size;
    unsigned long long addr;
    unsigned long long len;
    unsigned int type;
} __attribute__((packed));

typedef struct multiboot_info multiboot_info_t;

typedef struct multiboot_module {
    unsigned int mod_start;
    unsigned int mod_end;
    unsigned int cmdline;
    unsigned int reserved;
} __attribute__((packed)) multiboot_module_t;

#endif /* INCLUDE_MULTIBOOT_H */
