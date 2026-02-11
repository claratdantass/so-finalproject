# so-finalproject

Academic OS project based on the [Little OS Book](https://littleosbook.github.io/). This repo covers **Chapters 2–4**: boot loader, kernel bootstrap in assembly, transition to C, and output drivers (VGA framebuffer + serial port), with GRUB 2 boot and QEMU.

---

## Dependencies (macOS)

The project was developed and tested on **macOS**. The setup targets **x86 32-bit** (`i686-elf`). You need the tools below to compile the kernel, build the boot image, and run it in a VM.

- **Homebrew** — package manager for installing the rest
- **make** — runs the build (often already installed)
- **nasm** — assembler for the assembly code (elf32)
- **qemu** — emulator to run the OS (e.g. `qemu-system-i386`). We went with QEMU after looking into options; it tends to work better on Mac for this kind of project.
- **xorriso** — used by `grub-mkrescue` to create the ISO
- **i686-elf-grub** — GRUB 2 for ELF i686 (includes `i686-elf-grub-mkrescue`)
- **i686-elf-gcc** — cross GCC compiler for i686-elf (compiles C kernel code in freestanding mode)
- **i686-elf-binutils** — cross binutils for i686-elf (includes `i686-elf-ld`)

### Installation (macOS)

Install [Homebrew](https://brew.sh/) if needed, then:

```bash
brew install make nasm qemu xorriso i686-elf-gcc i686-elf-grub i686-elf-binutils
```

---

## Project layout

- `src/loader.s` — bootstrap: Multiboot header, stack setup, calls `kmain` (assembly)
- `src/io.s` — `outb`/`inb` assembly wrappers for I/O port access
- `src/kmain.c` — kernel entry point in C
- `src/drivers/fb.c` — VGA framebuffer driver (text output, cursor, scrolling)
- `src/drivers/serial.c` — serial port (COM1) driver for logging
- `include/` — header files (`io.h`, `fb.h`, `serial.h`)
- `link.ld` — linker script for the kernel
- `build/` — output directory (`.o`, `kernel.elf`, `os.iso`)
- `iso/boot/grub/grub.cfg` — GRUB menu config
- `com1.out` — serial port log (created by `make run`)

## Usage

- **`make`** or **`make kernel`** — builds `build/kernel.elf`
- **`make iso`** — builds `build/os.iso`
- **`make run`** — runs the OS in QEMU
- **`make clean`** — removes build artifacts
