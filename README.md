# so-finalproject

Academic OS project based on the [Little OS Book](https://littleosbook.github.io/). This repo covers **Chapter 2**: boot loader and minimal kernel in assembly, with GRUB 2 boot and QEMU.

---

## Dependencies (macOS)

The project was developed and tested on **macOS**. The setup targets **x86 32-bit** (`i686-elf`). You need the tools below to compile the kernel, build the boot image, and run it in a VM.

- **Homebrew** — package manager for installing the rest
- **make** — runs the build (often already installed)
- **nasm** — assembler for the assembly code (elf32)
- **qemu** — emulator to run the OS (e.g. `qemu-system-i386`). We went with QEMU after looking into options; it tends to work better on Mac for this kind of project.
- **xorriso** — used by `grub-mkrescue` to create the ISO
- **i686-elf-grub** — GRUB 2 for ELF i686 (includes `i686-elf-grub-mkrescue`)
- **i686-elf-binutils** — cross binutils for i686-elf (includes `i686-elf-ld`)

### Installation (macOS)

Install [Homebrew](https://brew.sh/) if needed, then:

```bash
brew install make nasm qemu xorriso i686-elf-grub i686-elf-binutils
```

---

## Project layout

- `src/loader.s` — entry point and Multiboot header (assembly)
- `link.ld` — linker script for the kernel
- `build/` — output directory (`.o`, `kernel.elf`, `os.iso`)
- `iso/boot/grub/grub.cfg` — GRUB menu config

## Usage

- **`make`** or **`make kernel`** — builds `build/kernel.elf`
- **`make iso`** — builds `build/os.iso`
- **`make run`** — runs the OS in QEMU
- **`make clean`** — removes build artifacts
