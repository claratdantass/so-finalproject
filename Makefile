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

PAGING_SRC := $(SRC_DIR)/paging.s
PAGING_OBJ := $(BUILD_DIR)/paging.o

PFA_SRC    := $(SRC_DIR)/pfa.c
PFA_OBJ    := $(BUILD_DIR)/pfa.o

KHEAP_SRC  := $(SRC_DIR)/kheap.c
KHEAP_OBJ  := $(BUILD_DIR)/kheap.o

KUTIL_SRC  := $(SRC_DIR)/kutil.c
KUTIL_OBJ  := $(BUILD_DIR)/kutil.o

TSS_SRC    := $(SRC_DIR)/tss.c
TSS_OBJ    := $(BUILD_DIR)/tss.o

PAGING4K_SRC := $(SRC_DIR)/paging4k.c
PAGING4K_OBJ := $(BUILD_DIR)/paging4k.o

FS_SRC     := $(SRC_DIR)/fs.c
FS_OBJ     := $(BUILD_DIR)/fs.o

SYSCALL_ASM     := $(SRC_DIR)/syscall_handler.s
SYSCALL_ASM_OBJ := $(BUILD_DIR)/syscall_handler.o

SYSCALL_SRC := $(SRC_DIR)/syscall.c
SYSCALL_OBJ := $(BUILD_DIR)/syscall.o

CTXSW_ASM  := $(SRC_DIR)/context_switch.s
CTXSW_OBJ  := $(BUILD_DIR)/context_switch.o

PROCESS_SRC := $(SRC_DIR)/process.c
PROCESS_OBJ := $(BUILD_DIR)/process.o

PIT_SRC    := $(SRC_DIR)/pit.c
PIT_OBJ    := $(BUILD_DIR)/pit.o

UMODE_ASM  := $(SRC_DIR)/usermode.s
UMODE_OBJ  := $(BUILD_DIR)/usermode.o

# user-mode program (flat binary loaded as a GRUB module)
PROG_DIR       := programs
PROG_START_SRC := $(PROG_DIR)/start.s
PROG_START_OBJ := $(BUILD_DIR)/prog_start.o
PROG_MAIN_SRC  := $(PROG_DIR)/program.c
PROG_MAIN_OBJ  := $(BUILD_DIR)/prog_program.o
PROG_HELLO_SRC := $(PROG_DIR)/hello.c
PROG_HELLO_OBJ := $(BUILD_DIR)/prog_hello.o
PROG_LINK      := $(PROG_DIR)/link.ld

# rootfs directory and initrd image
ROOTFS_DIR     := $(BUILD_DIR)/rootfs
INITRD         := $(ISO_DIR)/modules/initrd

PROG_CFLAGS := -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
               -nostartfiles -nodefaultlibs -fno-pie -Wall -Wextra -Werror -c

# all object files needed for linking
ALL_OBJS := $(LOADER_OBJ) $(IO_OBJ) $(GDT_OBJ) $(GDT_C_OBJ) \
            $(IDT_ASM_OBJ) $(IDT_OBJ) $(INT_OBJ) \
            $(PIC_OBJ) $(KBD_OBJ) \
            $(FB_OBJ) $(SERIAL_OBJ) \
            $(PAGING_OBJ) $(PFA_OBJ) $(KHEAP_OBJ) $(KUTIL_OBJ) \
            $(TSS_OBJ) $(PAGING4K_OBJ) $(FS_OBJ) \
            $(SYSCALL_ASM_OBJ) $(SYSCALL_OBJ) \
            $(CTXSW_OBJ) $(PROCESS_OBJ) $(PIT_OBJ) $(UMODE_OBJ) \
            $(KMAIN_OBJ)

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

$(PAGING_OBJ): $(PAGING_SRC) | $(BUILD_DIR)
	$(NASM) -f elf32 $(PAGING_SRC) -o $@

$(PFA_OBJ): $(PFA_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(KHEAP_OBJ): $(KHEAP_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(KUTIL_OBJ): $(KUTIL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(TSS_OBJ): $(TSS_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(PAGING4K_OBJ): $(PAGING4K_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(FS_OBJ): $(FS_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(SYSCALL_ASM_OBJ): $(SYSCALL_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $(SYSCALL_ASM) -o $@

$(SYSCALL_OBJ): $(SYSCALL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(CTXSW_OBJ): $(CTXSW_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $(CTXSW_ASM) -o $@

$(PROCESS_OBJ): $(PROCESS_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(PIT_OBJ): $(PIT_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(UMODE_OBJ): $(UMODE_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $(UMODE_ASM) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# assemble/compile user program as a flat binary
$(PROG_START_OBJ): $(PROG_START_SRC) | $(BUILD_DIR)
	$(NASM) -f elf32 $(PROG_START_SRC) -o $@

$(PROG_MAIN_OBJ): $(PROG_MAIN_SRC) | $(BUILD_DIR)
	$(CC) $(PROG_CFLAGS) $< -o $@

$(PROG_HELLO_OBJ): $(PROG_HELLO_SRC) | $(BUILD_DIR)
	$(CC) $(PROG_CFLAGS) $< -o $@

# build rootfs directory with user programs, then pack into initrd
$(ROOTFS_DIR): | $(BUILD_DIR)
	mkdir -p $(ROOTFS_DIR)

$(ROOTFS_DIR)/program: $(PROG_START_OBJ) $(PROG_MAIN_OBJ) $(PROG_LINK) | $(ROOTFS_DIR)
	$(LD) -T $(PROG_LINK) -melf_i386 $(PROG_START_OBJ) $(PROG_MAIN_OBJ) -o $@

$(ROOTFS_DIR)/hello: $(PROG_START_OBJ) $(PROG_HELLO_OBJ) $(PROG_LINK) | $(ROOTFS_DIR)
	$(LD) -T $(PROG_LINK) -melf_i386 $(PROG_START_OBJ) $(PROG_HELLO_OBJ) -o $@

$(INITRD): $(ROOTFS_DIR)/program $(ROOTFS_DIR)/hello tools/mkfs.py
	mkdir -p $(ISO_DIR)/modules
	python3 tools/mkfs.py $(ROOTFS_DIR) $@

# make bootable ISO
iso: $(OS_ISO)

$(OS_ISO): kernel $(INITRD)
	cp $(KERNEL_ELF) $(ISO_BOOT_KERNEL)
	$(GRUB_MKRESCUE) -o $(OS_ISO) $(ISO_DIR)

# run in QEMU (serial output goes to com1.out)
run: iso
	$(QEMU) -cdrom $(OS_ISO) -serial file:com1.out

# run in QEMU with monitor on stdio (type 'info registers' to inspect CPU state)
debug: iso
	$(QEMU) -cdrom $(OS_ISO) -serial file:com1.out -monitor stdio -no-reboot -no-shutdown

# remove build output and iso
clean:
	rm -f $(ALL_OBJS) $(KERNEL_ELF) $(OS_ISO) $(ISO_BOOT_KERNEL) $(INITRD) \
	      $(PROG_START_OBJ) $(PROG_MAIN_OBJ) $(PROG_HELLO_OBJ)
	rm -rf $(ROOTFS_DIR)
