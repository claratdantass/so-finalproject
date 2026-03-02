# so-finalproject

Academic OS project based on the [Little OS Book](https://littleosbook.github.io/). This repo covers **Chapters 2–5**: boot loader, minimal kernel bootstrap, transition to C, and segmentation (GDT), with GRUB 2 boot and QEMU.

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
- `src/gdt.c` — GDT descriptors and init (segmentation)
- `src/gdt.s` — assembly: load GDT, reload segment registers
- `src/gdt.h` — GDT init declaration
- `link.ld` — linker script for the kernel
- `build/` — output directory (`.o`, `kernel.elf`, `os.iso`)
- `iso/boot/grub/grub.cfg` — GRUB menu config
- `com1.out` — serial port log (created by `make run`)

## Usage

- **`make`** or **`make kernel`** — builds `build/kernel.elf`
- **`make iso`** — builds `build/os.iso`
- **`make run`** — runs the OS in QEMU
- **`make clean`** — removes build artifacts

---

## Testing the GDT (segments)

After pulling the repo and installing dependencies, you can check that segmentation is working like this.

You need **i386-elf-gdb** (optional): `brew install i386-elf-gdb`.

**Terminal 1** — start QEMU in debug mode and leave it running:

```bash
make iso
qemu-system-i386 -cdrom build/os.iso -s -S
```

**Terminal 2** — run GDB:

```bash
i386-elf-gdb build/kernel.elf
```

Inside GDB, run **one command at a time** (press Enter after each). Do not paste the whole block at once.

| Step | Command |
|------|---------|
| 1 | `target remote localhost:1234` |
| 2 | `break gdt_load` |
| 3 | `continue` |
| 4 | `finish` |
| 5 | `i r cs ds ss es fs gs` |

You stop at the start of `gdt_load`, then `finish` runs until the function returns. After step 5 you should see **cs = 0x8** and **ds = ss = es = fs = gs = 0x10**.

**Important:** Terminal 1 must be running QEMU with `-s -S` *before* you run step 1 in GDB. Otherwise you get “cannot resolve name” or “no registers”.
