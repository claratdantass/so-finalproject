# Builds kernel.elf, packs it into an ISO, runs in QEMU. Tested on macOS, target x86 32-bit.

# paths
SRC_DIR   := src
BUILD_DIR := build
ISO_DIR   := iso
INC_DIR   := include

LOADER_SRC := $(SRC_DIR)/loader.s
LOADER_OBJ := $(BUILD_DIR)/loader.o

IO_SRC     := $(SRC_DIR)/io.s
IO_OBJ     := $(BUILD_DIR)/io.o

KMAIN_SRC  := $(SRC_DIR)/kmain.c
KMAIN_OBJ  := $(BUILD_DIR)/kmain.o

FB_SRC     := $(SRC_DIR)/drivers/fb.c
FB_OBJ     := $(BUILD_DIR)/fb.o

SERIAL_SRC := $(SRC_DIR)/drivers/serial.c
SERIAL_OBJ := $(BUILD_DIR)/serial.o

OBJECTS    := $(LOADER_OBJ) $(IO_OBJ) $(KMAIN_OBJ) $(FB_OBJ) $(SERIAL_OBJ)

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
          -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I$(INC_DIR)

# targets
.PHONY: all kernel iso run clean
all: kernel

# default: build the kernel
kernel: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJECTS) $(LINK_SCRIPT)
	$(LD) -T $(LINK_SCRIPT) -o $@ $(OBJECTS)

# assemble loader.s to elf32 object file
$(LOADER_OBJ): $(LOADER_SRC) | $(BUILD_DIR)
	$(NASM) -f elf32 $(LOADER_SRC) -o $@

# assemble io.s to elf32 object file
$(IO_OBJ): $(IO_SRC) | $(BUILD_DIR)
	$(NASM) -f elf32 $(IO_SRC) -o $@

# compile kmain.c to object file
$(KMAIN_OBJ): $(KMAIN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# compile framebuffer driver
$(FB_OBJ): $(FB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# compile serial driver
$(SERIAL_OBJ): $(SERIAL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# make bootable ISO
iso: $(OS_ISO)

$(OS_ISO): kernel
	cp $(KERNEL_ELF) $(ISO_BOOT_KERNEL)
	$(GRUB_MKRESCUE) -o $(OS_ISO) $(ISO_DIR)

# run in QEMU (serial output goes to com1.out)
run: iso
	$(QEMU) -cdrom $(OS_ISO) -serial file:com1.out

# remove build output and iso
clean:
	rm -f $(OBJECTS) $(KERNEL_ELF) $(OS_ISO) $(ISO_BOOT_KERNEL)
