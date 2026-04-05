# Simple Read-Only File System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a simple read-only in-memory file system that packages multiple user programs into a single GRUB module, allowing the kernel to find and load programs by name.

**Architecture:** A build-time Python tool (`mkfs.py`) packs files from a directory into a flat binary image with a header + file table + data. The kernel parses this image at boot to find programs by name. The GRUB module changes from a single raw program binary to a filesystem image containing all user programs. The kernel's user-mode loader is updated to look up the program to run from the filesystem.

**Tech Stack:** Python 3 (build tool), i686-elf-gcc, NASM, QEMU

---

## Filesystem Format (SOFS — SO File System)

```
Offset 0:   [fs_header]           8 bytes (magic + num_files)
Offset 8:   [fs_file_entry 0]    72 bytes (name[64] + offset + size)
            [fs_file_entry 1]    72 bytes
            ...
            [fs_file_entry N-1]  72 bytes
Data area:  [file 0 data]        (4-byte aligned)
            [file 1 data]        (4-byte aligned)
            ...
```

## File Structure

**New files:**

| File | Responsibility |
|------|---------------|
| `include/fs.h` | Filesystem structures and API declarations |
| `src/fs.c` | Parse filesystem image, find files by name, get file data/size |
| `tools/mkfs.py` | Build-time tool: pack a directory into a SOFS image |

**Modified files:**

| File | Change |
|------|--------|
| `include/kutil.h` | Add `strcmp`, `strlen` declarations |
| `src/kutil.c` | Add `strcmp`, `strlen` implementations |
| `src/kmain.c` | Use filesystem to find and load user program instead of raw module |
| `iso/boot/grub/grub.cfg` | Change module path from `/modules/program` to `/modules/initrd` |
| `Makefile` | Build user programs into rootfs, run mkfs.py, load initrd as module |

---

### Task 1: Add String Utilities

**Files:**
- Modify: `include/kutil.h`
- Modify: `src/kutil.c`

- [ ] **Step 1: Add `strcmp` and `strlen` to `include/kutil.h`**

Add after the `memcpy` declaration:

```c
int strcmp(const char *s1, const char *s2);
unsigned int strlen(const char *s);
```

- [ ] **Step 2: Add `strcmp` and `strlen` to `src/kutil.c`**

Add at the end of the file:

```c
int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

unsigned int strlen(const char *s)
{
    unsigned int len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}
```

- [ ] **Step 3: Build to verify**

Run: `make clean && make kernel`
Expected: Compiles with no errors.

- [ ] **Step 4: Commit**

```bash
git add include/kutil.h src/kutil.c
git commit -m "feat: add strcmp and strlen to kernel utilities"
```

---

### Task 2: Filesystem Header and Kernel Driver

**Files:**
- Create: `include/fs.h`
- Create: `src/fs.c`
- Modify: `Makefile`

- [ ] **Step 1: Create `include/fs.h`**

```c
#ifndef INCLUDE_FS_H
#define INCLUDE_FS_H

#define FS_MAGIC     0x534F4653U
#define FS_MAX_NAME  64

struct fs_header {
    unsigned int magic;
    unsigned int num_files;
};

struct fs_file_entry {
    char name[FS_MAX_NAME];
    unsigned int offset;
    unsigned int size;
};

struct fs_instance {
    unsigned int base_addr;
    struct fs_header *header;
    struct fs_file_entry *entries;
};

int  fs_init(struct fs_instance *fs, unsigned int base_addr);
int  fs_num_files(struct fs_instance *fs);
int  fs_find(struct fs_instance *fs, const char *name);
unsigned int fs_file_data(struct fs_instance *fs, int index);
unsigned int fs_file_size(struct fs_instance *fs, int index);
const char  *fs_file_name(struct fs_instance *fs, int index);

#endif /* INCLUDE_FS_H */
```

- [ ] **Step 2: Create `src/fs.c`**

```c
#include "fs.h"
#include "kutil.h"

int fs_init(struct fs_instance *fs, unsigned int base_addr)
{
    fs->base_addr = base_addr;
    fs->header = (struct fs_header *)base_addr;

    if (fs->header->magic != FS_MAGIC) {
        fs->header = (struct fs_header *)0;
        fs->entries = (struct fs_file_entry *)0;
        return -1;
    }

    fs->entries = (struct fs_file_entry *)(base_addr + sizeof(struct fs_header));
    return 0;
}

int fs_num_files(struct fs_instance *fs)
{
    if (!fs->header) {
        return 0;
    }
    return (int)fs->header->num_files;
}

int fs_find(struct fs_instance *fs, const char *name)
{
    unsigned int i;

    if (!fs->header) {
        return -1;
    }

    for (i = 0; i < fs->header->num_files; i++) {
        if (strcmp(fs->entries[i].name, name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

unsigned int fs_file_data(struct fs_instance *fs, int index)
{
    return fs->base_addr + fs->entries[index].offset;
}

unsigned int fs_file_size(struct fs_instance *fs, int index)
{
    return fs->entries[index].size;
}

const char *fs_file_name(struct fs_instance *fs, int index)
{
    return fs->entries[index].name;
}
```

- [ ] **Step 3: Add fs to Makefile**

Add after the PAGING4K variables:

```makefile
FS_SRC     := $(SRC_DIR)/fs.c
FS_OBJ     := $(BUILD_DIR)/fs.o
```

Add `$(FS_OBJ)` to `ALL_OBJS` (after `$(PAGING4K_OBJ)`).

Add the build rule:

```makefile
$(FS_OBJ): $(FS_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@
```

- [ ] **Step 4: Build to verify**

Run: `make clean && make kernel`
Expected: Compiles with no errors.

- [ ] **Step 5: Commit**

```bash
git add include/fs.h src/fs.c Makefile
git commit -m "feat: add simple read-only filesystem driver (SOFS)"
```

---

### Task 3: Build-Time Filesystem Tool

**Files:**
- Create: `tools/mkfs.py`

- [ ] **Step 1: Create `tools/mkfs.py`**

```python
#!/usr/bin/env python3
"""
mkfs.py — Build a SOFS (SO File System) image from a directory.

Format:
  [fs_header: magic(4) + num_files(4)]
  [fs_file_entry[0]: name(64) + offset(4) + size(4)]
  ...
  [fs_file_entry[N-1]]
  [file 0 data (4-byte aligned)]
  ...
  [file N-1 data (4-byte aligned)]
"""

import struct
import os
import sys

FS_MAGIC = 0x534F4653
MAX_NAME = 64
HEADER_SIZE = 8
ENTRY_SIZE = MAX_NAME + 4 + 4  # 72 bytes


def align4(n):
    return (n + 3) & ~3


def build_fs(input_dir, output_path):
    filenames = sorted(
        f for f in os.listdir(input_dir)
        if os.path.isfile(os.path.join(input_dir, f))
    )

    if not filenames:
        print("error: no files found in", input_dir, file=sys.stderr)
        sys.exit(1)

    num_files = len(filenames)
    data_start = HEADER_SIZE + ENTRY_SIZE * num_files

    file_contents = []
    entries = []
    current_offset = data_start

    for fname in filenames:
        path = os.path.join(input_dir, fname)
        with open(path, "rb") as f:
            data = f.read()

        name_bytes = fname.encode("ascii")[:MAX_NAME - 1]
        name_bytes = name_bytes + b"\x00" * (MAX_NAME - len(name_bytes))

        entries.append((name_bytes, current_offset, len(data)))
        file_contents.append(data)
        current_offset = align4(current_offset + len(data))

    with open(output_path, "wb") as out:
        out.write(struct.pack("<II", FS_MAGIC, num_files))

        for name_bytes, offset, size in entries:
            out.write(name_bytes)
            out.write(struct.pack("<II", offset, size))

        for data in file_contents:
            out.write(data)
            padding = align4(len(data)) - len(data)
            if padding:
                out.write(b"\x00" * padding)

    total = os.path.getsize(output_path)
    print(f"mkfs: {num_files} file(s), {total} bytes -> {output_path}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_dir> <output_file>", file=sys.stderr)
        sys.exit(1)
    build_fs(sys.argv[1], sys.argv[2])
```

- [ ] **Step 2: Make it executable**

```bash
chmod +x tools/mkfs.py
```

- [ ] **Step 3: Commit**

```bash
git add tools/mkfs.py
git commit -m "feat: add mkfs.py build tool for SOFS filesystem images"
```

---

### Task 4: Update Build System and GRUB Config

**Files:**
- Modify: `Makefile`
- Modify: `iso/boot/grub/grub.cfg`

- [ ] **Step 1: Update `iso/boot/grub/grub.cfg`**

Change `module /modules/program` to `module /modules/initrd`:

```
set timeout=0
set default=0

menuentry "os" {
  multiboot /boot/kernel.elf
  module /modules/initrd
  boot
}
```

- [ ] **Step 2: Update `Makefile` — build rootfs and initrd**

Replace the user program section. Remove:

```makefile
PROG_BIN       := $(ISO_DIR)/modules/program
```

And the old `$(PROG_BIN)` rule. Replace with the rootfs + initrd build:

```makefile
ROOTFS_DIR     := $(BUILD_DIR)/rootfs
INITRD         := $(ISO_DIR)/modules/initrd

$(ROOTFS_DIR): | $(BUILD_DIR)
	mkdir -p $(ROOTFS_DIR)

$(ROOTFS_DIR)/program: $(PROG_START_OBJ) $(PROG_MAIN_OBJ) $(PROG_LINK) | $(ROOTFS_DIR)
	$(LD) -T $(PROG_LINK) -melf_i386 $(PROG_START_OBJ) $(PROG_MAIN_OBJ) -o $@

$(INITRD): $(ROOTFS_DIR)/program tools/mkfs.py
	mkdir -p $(ISO_DIR)/modules
	python3 tools/mkfs.py $(ROOTFS_DIR) $@
```

Update the ISO target to depend on `$(INITRD)` instead of `$(PROG_BIN)`:

```makefile
$(OS_ISO): kernel $(INITRD)
	cp $(KERNEL_ELF) $(ISO_BOOT_KERNEL)
	$(GRUB_MKRESCUE) -o $(OS_ISO) $(ISO_DIR)
```

Update the clean target:

```makefile
clean:
	rm -f $(ALL_OBJS) $(KERNEL_ELF) $(OS_ISO) $(ISO_BOOT_KERNEL) $(INITRD) \
	      $(PROG_START_OBJ) $(PROG_MAIN_OBJ)
	rm -rf $(ROOTFS_DIR)
```

- [ ] **Step 3: Build the initrd and verify**

Run: `make clean && make iso`
Expected: `mkfs: 1 file(s), ... bytes -> iso/modules/initrd` and ISO produced.

Verify the initrd contains the magic:

```bash
xxd iso/modules/initrd | head -5
```

Expected: First 4 bytes are `5346 4f53` (SOFS in little-endian: `53 4f 46 53`).

- [ ] **Step 4: Commit**

```bash
git add Makefile iso/boot/grub/grub.cfg tools/mkfs.py
git commit -m "feat: build filesystem image (initrd) from rootfs directory"
```

---

### Task 5: Update Kernel to Load Programs from Filesystem

**Files:**
- Modify: `src/kmain.c`

- [ ] **Step 1: Replace module-loading code with filesystem lookup**

Replace the entire `src/kmain.c` with:

```c
#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "fb.h"
#include "fs.h"
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

static struct fs_instance rootfs;

static void load_and_run(const char *name)
{
    int idx;
    unsigned int prog_data;
    unsigned int prog_size;
    unsigned int code_pages;
    unsigned int *user_dir;
    unsigned int stack_frame;
    unsigned int code_frame;
    unsigned int offset;
    unsigned int copy_len;
    unsigned int i;

    idx = fs_find(&rootfs, name);
    if (idx < 0) {
        fb_write("ERROR: program '", 16);
        fb_write(name, strlen(name));
        fb_write("' not found.\n", 13);
        return;
    }

    prog_data = fs_file_data(&rootfs, idx);
    prog_size = fs_file_size(&rootfs, idx);
    code_pages = (prog_size + PAGE_SIZE - 1U) / PAGE_SIZE;

    user_dir = paging_create_kernel_dir();
    if (user_dir == (unsigned int *)0) {
        fb_write("ERROR: Cannot alloc page dir.\n", 30);
        return;
    }

    for (i = 0; i < code_pages; i++) {
        code_frame = pfa_alloc();
        if (code_frame == 0U) {
            fb_write("ERROR: Cannot alloc code frame.\n", 32);
            return;
        }
        offset = i * PAGE_SIZE;
        copy_len = prog_size - offset;
        if (copy_len > PAGE_SIZE) {
            copy_len = PAGE_SIZE;
        }
        memcpy((void *)code_frame, (void *)(prog_data + offset), copy_len);
        if (copy_len < PAGE_SIZE) {
            memset((void *)(code_frame + copy_len), 0, PAGE_SIZE - copy_len);
        }
        paging_map_4k(user_dir, USER_CODE_VIRT + offset, code_frame,
                       PTE_PRESENT | PTE_RW | PTE_USER);
    }

    stack_frame = pfa_alloc();
    if (stack_frame == 0U) {
        fb_write("ERROR: Cannot alloc stack.\n", 27);
        return;
    }
    memset((void *)stack_frame, 0, PAGE_SIZE);
    paging_map_4k(user_dir, USER_STACK_VIRT, stack_frame,
                   PTE_PRESENT | PTE_RW | PTE_USER);

    fb_write("Entering user mode...\n", 22);
    enter_usermode(USER_CODE_VIRT, USER_STACK_TOP, (unsigned int)user_dir);
}

void kmain(unsigned int multiboot_magic,
           unsigned int multiboot_info_addr,
           unsigned int kernel_physical_start,
           unsigned int kernel_physical_end)
{
    multiboot_info_t *mbinfo = (multiboot_info_t *)multiboot_info_addr;
    multiboot_module_t *mod;

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
    pfa_reserve(mod->mod_start, mod->mod_end);

    if (fs_init(&rootfs, mod->mod_start) != 0) {
        fb_write("ERROR: Invalid filesystem.\n", 27);
        goto halt;
    }

    fb_write("SOFS: ", 6);
    {
        int n = fs_num_files(&rootfs);
        char digit = '0' + (char)n;
        fb_write(&digit, 1);
    }
    fb_write(" file(s) loaded.\n", 17);

    load_and_run("program");

halt:
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
```

Key changes from the previous version:
- `#include "fs.h"` added
- `static struct fs_instance rootfs` — global filesystem instance
- `load_and_run(name)` — extracted function: finds a program in the FS, sets up user address space, enters user mode
- `kmain` now: reserves module memory, calls `fs_init` on it, prints file count, then calls `load_and_run("program")`
- The GRUB module is treated as a filesystem image, not a raw binary

- [ ] **Step 2: Build and run**

Run: `make clean && make iso && make run`
Expected: QEMU screen shows:

```
SOFS: 1 file(s) loaded.
Entering user mode...
```

- [ ] **Step 3: Commit**

```bash
git add src/kmain.c
git commit -m "feat: load user programs from SOFS filesystem image"
```

---

## Verification

After all tasks, the boot sequence is:

1. GRUB loads `kernel.elf` and the `initrd` module (filesystem image)
2. Kernel initializes hardware (GDT, TSS, paging, PIC, IDT, PFA, heap)
3. Kernel parses the initrd as a SOFS filesystem
4. Kernel prints "SOFS: N file(s) loaded."
5. Kernel calls `load_and_run("program")` which finds the program in the filesystem
6. User address space is created, program is copied and mapped
7. `iret` to ring 3 — "Entering user mode..."

## What Comes Next

This filesystem provides the foundation for the mini-shell:
- **System calls** (`write`, `read`, `yield`, `exit`, `spawn`) — programs can request kernel services
- **Process management** — PCBs, cooperative scheduler
- **Mini-shell** — reads commands, looks up programs in the filesystem, spawns them
- **Top viewer** — reads process list via syscall, displays in real-time
