# Builds kernel.elf, packs it into an ISO, runs in QEMU. Tested on macOS, target x86 32-bit.

# paths
SRC_DIR   := src
BUILD_DIR := build
ISO_DIR   := iso

LOADER_SRC := $(SRC_DIR)/loader.s
LOADER_OBJ := $(BUILD_DIR)/loader.o
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
OS_ISO     := $(BUILD_DIR)/os.iso

# kernel goes here so GRUB finds it at /boot/kernel.elf
ISO_BOOT_KERNEL := $(ISO_DIR)/boot/kernel.elf

LINK_SCRIPT := link.ld

# tools (i686-elf cross toolchain from Homebrew on mac)
NASM   := nasm
LD     := i686-elf-ld
GRUB_MKRESCUE := i686-elf-grub-mkrescue
QEMU   := qemu-system-i386

# targets
.PHONY: all kernel iso run clean
all: kernel

# default: build the kernel
kernel: $(KERNEL_ELF)

$(KERNEL_ELF): $(LOADER_OBJ) $(LINK_SCRIPT)
	$(LD) -T $(LINK_SCRIPT) -o $@ $(LOADER_OBJ)

# assemble loader.s to elf32 object file
$(LOADER_OBJ): $(LOADER_SRC) | $(BUILD_DIR)
	$(NASM) -f elf32 $(LOADER_SRC) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# make bootable ISO
iso: $(OS_ISO)

$(OS_ISO): kernel
	cp $(KERNEL_ELF) $(ISO_BOOT_KERNEL)
	$(GRUB_MKRESCUE) -o $(OS_ISO) $(ISO_DIR)

# run in QEMU
run: iso
	$(QEMU) -cdrom $(OS_ISO)

# remove build output and iso
clean:
	rm -f $(LOADER_OBJ) $(KERNEL_ELF) $(OS_ISO) $(ISO_BOOT_KERNEL)
