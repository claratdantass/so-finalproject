# Builds kernel.elf, packs it into an ISO, runs in QEMU. Tested on macOS, target x86 32-bit.

# paths
SRC_DIR   := src
BUILD_DIR := build
ISO_DIR   := iso
INC_DIR   := include

LOADER_SRC := $(SRC_DIR)/loader.s
LOADER_OBJ := $(BUILD_DIR)/loader.o

GDT_ASM   := $(SRC_DIR)/gdt.s
GDT_OBJ   := $(BUILD_DIR)/gdt_load.o

GDT_SRC   := $(SRC_DIR)/gdt.c
GDT_C_OBJ := $(BUILD_DIR)/gdt.o

IO_ASM     := $(SRC_DIR)/io.s
IO_OBJ     := $(BUILD_DIR)/io.o

IDT_ASM    := $(SRC_DIR)/idt.s
IDT_ASM_OBJ := $(BUILD_DIR)/idt_load.o

IDT_SRC    := $(SRC_DIR)/idt.c
IDT_OBJ    := $(BUILD_DIR)/idt.o

INT_ASM    := $(SRC_DIR)/interrupt_handlers.s
INT_OBJ    := $(BUILD_DIR)/interrupt_handlers.o

PIC_SRC    := $(SRC_DIR)/drivers/pic.c
PIC_OBJ    := $(BUILD_DIR)/pic.o

KBD_SRC    := $(SRC_DIR)/drivers/keyboard.c
KBD_OBJ    := $(BUILD_DIR)/keyboard.o

KMAIN_SRC  := $(SRC_DIR)/kmain.c
KMAIN_OBJ  := $(BUILD_DIR)/kmain.o

FB_SRC     := $(SRC_DIR)/drivers/fb.c
FB_OBJ     := $(BUILD_DIR)/fb.o

SERIAL_SRC := $(SRC_DIR)/drivers/serial.c
SERIAL_OBJ := $(BUILD_DIR)/serial.o

# user-mode program (flat binary loaded as a GRUB module)
PROG_DIR   := programs
PROG_SRC   := $(PROG_DIR)/program.s
PROG_BIN   := $(ISO_DIR)/modules/program

# all object files needed for linking
ALL_OBJS := $(LOADER_OBJ) $(IO_OBJ) $(GDT_OBJ) $(GDT_C_OBJ) \
            $(IDT_ASM_OBJ) $(IDT_OBJ) $(INT_OBJ) \
            $(PIC_OBJ) $(KBD_OBJ) \
            $(FB_OBJ) $(SERIAL_OBJ) $(KMAIN_OBJ)

KERNEL_ELF := $(BUILD_DIR)/kernel.elf
OS_ISO     := $(BUILD_DIR)/os.iso

# kernel goes here so GRUB finds it at /boot/kernel.elf
ISO_BOOT_KERNEL := $(ISO_DIR)/boot/kernel.elf

LINK_SCRIPT := link.ld

# tools (i686-elf cross toolchain from Homebrew on mac)
NASM   := nasm
CC     := i686-elf-gcc
LD     := i686-elf-ld
GRUB_MKRESCUE := i686-elf-grub-mkrescue
QEMU   := qemu-system-i386

# C compiler flags: freestanding kernel, no stdlib, all warnings as errors
CFLAGS := -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
          -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
          -I$(INC_DIR) -I$(SRC_DIR)

# targets
.PHONY: all kernel iso run clean
all: kernel

# default: build the kernel
kernel: $(KERNEL_ELF)

$(KERNEL_ELF): $(ALL_OBJS) $(LINK_SCRIPT)
	$(LD) -T $(LINK_SCRIPT) -o $@ $(ALL_OBJS)

# assemble loader.s to elf32 object file
$(LOADER_OBJ): $(LOADER_SRC) | $(BUILD_DIR)
	$(NASM) -f elf32 $(LOADER_SRC) -o $@

# assemble I/O port wrappers
$(IO_OBJ): $(IO_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $(IO_ASM) -o $@

# assemble gdt.s (lgdt and segment reload)
$(GDT_OBJ): $(GDT_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $(GDT_ASM) -o $@

# assemble idt.s (lidt wrapper)
$(IDT_ASM_OBJ): $(IDT_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $(IDT_ASM) -o $@

# assemble interrupt handler stubs
$(INT_OBJ): $(INT_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $(INT_ASM) -o $@

# compile C sources
$(KMAIN_OBJ): $(KMAIN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(GDT_C_OBJ): $(GDT_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(IDT_OBJ): $(IDT_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(FB_OBJ): $(FB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(SERIAL_OBJ): $(SERIAL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(PIC_OBJ): $(PIC_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(KBD_OBJ): $(KBD_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# assemble the user program as a flat binary
$(PROG_BIN): $(PROG_SRC)
	mkdir -p $(ISO_DIR)/modules
	$(NASM) -f bin $(PROG_SRC) -o $@

# make bootable ISO
iso: $(OS_ISO)

$(OS_ISO): kernel $(PROG_BIN)
	cp $(KERNEL_ELF) $(ISO_BOOT_KERNEL)
	$(GRUB_MKRESCUE) -o $(OS_ISO) $(ISO_DIR)

# run in QEMU (serial output goes to com1.out)
run: iso
	$(QEMU) -cdrom $(OS_ISO) -serial file:com1.out

# remove build output and iso
clean:
	rm -f $(ALL_OBJS) $(KERNEL_ELF) $(OS_ISO) $(ISO_BOOT_KERNEL) $(PROG_BIN)
