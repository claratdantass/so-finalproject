# so-finalproject

Academic OS project based on the [Little OS Book](https://littleosbook.github.io/). Built from scratch for **x86 32-bit** (`i686-elf`), covering Chapters 2–4, 7–14: boot loader, C kernel entry, VGA/serial output, segmentation (GDT), interrupts (IDT + PIC), paging, memory allocators, user mode (ring 3), a simple read-only file system, system calls, and cooperative + preemptive multitasking.

---

## Features

| Chapter | Feature | Status |
|---------|---------|--------|
| 2–3 | Multiboot boot, C kernel entry, stack setup | Done |
| 4 | VGA framebuffer + serial port drivers | Done |
| 5 | GDT with ring 0 and ring 3 segments + TSS | Done |
| 6 | IDT, PIC remapping, keyboard IRQ handler | Done |
| 7 | GRUB module loading | Done |
| 8 | Identity-mapped paging (4 MiB PSE pages) | Done |
| 9 | Bitmap page frame allocator | Done |
| 10 | K&R-style kernel heap (`kmalloc`/`kfree`) | Done |
| 11 | **User mode** — ring 3 execution via `iret`, per-process 4 KiB page tables, TSS stack switching | Done |
| 12 | **File system** — SOFS read-only in-memory filesystem, build-time `mkfs.py` tool, programs loaded by name | Done |
| 13 | **System calls** — `int 0x80` with DPL=3 IDT gate, dedicated assembly handler, C dispatcher | Done |
| 14 | **Multitasking** — PCBs, round-robin scheduler, cooperative yield, PIT-based preemptive scheduling (50 Hz) | Done |

---

## Dependencies (macOS)

The project targets **x86 32-bit** and was developed on **macOS**.

- **Homebrew** — package manager
- **make** — build automation
- **nasm** — assembler (elf32)
- **qemu** — emulator (`qemu-system-i386`)
- **xorriso** — used by `grub-mkrescue` to create the ISO
- **i686-elf-grub** — GRUB 2 for ELF i686
- **i686-elf-gcc** — cross GCC compiler (freestanding)
- **i686-elf-binutils** — cross binutils (`i686-elf-ld`)
- **Python 3** — runs the `mkfs.py` filesystem build tool

### Installation

```bash
brew install make nasm qemu xorriso i686-elf-gcc i686-elf-grub i686-elf-binutils
```

---

## Project layout

### Kernel source (`src/`)

| File | Description |
|------|-------------|
| `loader.s` | Multiboot header, stack setup, calls `kmain` |
| `kmain.c` | Kernel entry: init hardware, parse filesystem, create processes, start scheduler |
| `gdt.c` / `gdt.s` | GDT with 6 entries (null, kernel code/data, user code/data, TSS) |
| `idt.c` / `idt.s` | IDT setup, interrupt gates, `int 0x80` registered with DPL=3 |
| `interrupt_handlers.s` | ISR stubs for interrupts 0–47 (CPU exceptions + IRQs) |
| `syscall_handler.s` | `int 0x80` entry: saves registers, calls C dispatcher, restores, `iret` |
| `syscall.c` | Syscall dispatcher: `SYS_WRITE`, `SYS_EXIT`, `SYS_YIELD`, `SYS_SPAWN` |
| `context_switch.s` | `switch_context` (saves/restores process state), `jump_to_process`, `process_start_trampoline` |
| `process.c` | Process table, `process_create`, round-robin `schedule`, `scheduler_start` |
| `usermode.s` | `enter_usermode`: switches page directory, sets user segments, `iret` to ring 3 |
| `tss.c` | TSS init, load (`ltr`), kernel stack update on context switch |
| `paging.s` | Identity-mapped paging bootstrap (4 MiB PSE pages) |
| `paging4k.c` | 4 KiB page table management, PSE page splitting, per-process page directories |
| `pfa.c` | Bitmap-based page frame allocator (from multiboot memory map) |
| `kheap.c` | K&R-style kernel heap (`kmalloc`/`kfree`) |
| `fs.c` | SOFS filesystem driver: parse image, find files by name |
| `pit.c` | PIT driver (50 Hz timer for preemptive scheduling) |
| `io.s` | `outb`/`inb` I/O port wrappers |
| `kutil.c` | `memset`, `memcpy`, `strcmp`, `strlen` |

### Drivers (`src/drivers/`)

| File | Description |
|------|-------------|
| `fb.c` | VGA text-mode framebuffer (write, scroll, cursor) |
| `serial.c` | Serial port (COM1) driver |
| `pic.c` | PIC remapping (IRQs → interrupts 32–47) + EOI |
| `keyboard.c` | Keyboard IRQ handler, scan-code → ASCII |

### Headers (`include/`)

`gdt.h`, `tss.h`, `idt.h`, `fb.h`, `serial.h`, `io.h`, `multiboot.h`, `paging.h`, `paging4k.h`, `pfa.h`, `kheap.h`, `kutil.h`, `pic.h`, `keyboard.h`, `fs.h`, `syscall.h`, `process.h`, `pit.h`, `usermode.h`

### User programs (`programs/`)

| File | Description |
|------|-------------|
| `start.s` | Entry stub: `_start` calls `main`, infinite loop safety |
| `link.ld` | Linker script: flat binary at VMA 0 |
| `syscall.h` | User-side syscall wrappers (`write`, `exit`, `yield`, `spawn`) via inline `int $0x80` |
| `program.c` | Test program A: prints messages, yields, exits |
| `hello.c` | Test program B: prints messages, yields, exits |

### Build tools

| File | Description |
|------|-------------|
| `tools/mkfs.py` | Packs a directory into a SOFS filesystem image (build-time tool) |
| `Makefile` | Builds kernel, user programs, rootfs, initrd, ISO |
| `link.ld` | Kernel linker script |

### Build output

| Path | Description |
|------|-------------|
| `build/` | Object files, `kernel.elf`, `os.iso` |
| `build/rootfs/` | Compiled user programs before packing |
| `iso/boot/grub/grub.cfg` | GRUB config (loads kernel + initrd module) |
| `iso/modules/initrd` | SOFS filesystem image with all user programs |

---

## Usage

```bash
make            # build kernel.elf
make iso        # build bootable os.iso (kernel + initrd)
make run        # run in QEMU
make debug      # run in QEMU with monitor on stdio (-no-reboot -no-shutdown)
make clean      # remove build artifacts
```

---

## System calls

User programs invoke system calls via `int $0x80`. Arguments go in registers (Linux-like convention):

| # | Name | Registers | Description |
|---|------|-----------|-------------|
| 1 | `SYS_WRITE` | eax=1, ebx=buffer, ecx=length | Write string to VGA framebuffer |
| 2 | `SYS_EXIT` | eax=2 | Terminate current process, schedule next |
| 3 | `SYS_YIELD` | eax=3 | Yield CPU to next ready process |
| 4 | `SYS_SPAWN` | eax=4, ebx=program name | Create new process from filesystem, returns PID |

Return value is placed in `eax`.

---

## SOFS file system

SOFS (SO File System) is a simple read-only in-memory filesystem:

```
[Header: magic "SFOS" (4 bytes) + file_count (4 bytes)]
[File entries: name (64 bytes) + offset (4 bytes) + size (4 bytes) each]
[File data: 4-byte aligned]
```

At build time, `tools/mkfs.py` packs all files from `build/rootfs/` into `iso/modules/initrd`. GRUB loads this as a multiboot module. The kernel parses it with `fs_init()` and looks up programs by name with `fs_find()`.

---

## Multitasking

The OS supports both **cooperative** and **preemptive** scheduling:

- **Cooperative**: processes call `yield()` to voluntarily give up the CPU
- **Preemptive**: the PIT fires at 50 Hz; the timer interrupt handler calls `schedule()` when interrupting user-mode code

Each process has:
- Its own **page directory** (isolated virtual address space)
- Its own **kernel stack** (for safe privilege transitions via TSS)
- A **PCB** (Process Control Block) in a fixed-size process table

Context switching is done via `switch_context` in assembly, which saves callee-saved registers + EFLAGS, switches ESP and CR3, and restores the new process's state. New processes bootstrap through `process_start_trampoline` which sets user data segments and `iret`s into ring 3.

---

## Boot sequence

1. GRUB loads `kernel.elf` and the `initrd` module
2. `loader.s` sets up the stack and calls `kmain`
3. `kmain` initializes: TSS → GDT → paging → serial → framebuffer → IDT → PFA → heap
4. Parses the initrd as a SOFS filesystem
5. Creates user processes from filesystem programs
6. Configures PIT at 50 Hz for preemptive scheduling
7. Calls `scheduler_start()` → first process enters user mode via trampoline → `iret`
8. Processes run in ring 3, communicate with kernel via `int 0x80`
9. Timer interrupts preempt user code; `schedule()` picks the next ready process
