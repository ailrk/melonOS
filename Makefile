ARCH ?= i686
CC = $(ARCH)-elf-gcc
LD = $(ARCH)-elf-ld
AS = nasm -f elf32
AR = ar rcs
CPP = cpp
HOSTCC = gcc
CFLAGS = -ffreestanding -g -nostdlib
CWARNS = -Wall -Wextra -fno-exceptions

B_DIR = boot
K_DIR = kernel
L_DIR = lib
M_DIR = melon
F_DIR = mkfs


BOOT = melonos-bootloader
KERNEL = melonos-kernel
MELONOS = melonos.img
MELONFS = melonfs.img

MKFS = mkfs.melonfs

LIBUTILS = libutils.a
LIBMELON = libmelon.a


QEMU = qemu-system-i386

.PHONY: boot kernel all

default: all
all: $(MELONOS)
boot: $(BOOT)
kernel: $(KERNEL)

$(MELONOS): $(BOOT) $(KERNEL)
	dd if=/dev/zero of=$(MELONOS) count=10000
	dd if=$(BOOT) of=$(MELONOS) conv=notrunc
	dd if=$(KERNEL) of=$(MELONOS) seek=20 conv=notrunc

$(MELONFS): $(MKFS)
	./$(MKFS) $(MELONFS)

.PHONY: clean qemu-debug copy echo
clean:
	find $(K_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	find $(B_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	find $(L_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	rm -rf *.o *.pp.* $(MELONOS) $(BOOT) $(KERNEL) $(LIBUTILS) $(LIBMELON)

echo:
	@echo 'CC $(CC)'
	@echo 'AS $(AS)'
	@echo 'MELONOS $(MELONOS)'
	@echo 'BOOT	$(BOOT)'
	@echo 'KERNEL $(KERNEL)'
	@echo 'K_OBJS $(K_OBJS)'
	@echo 'K_ASMFILES $(K_ASMFILES)'
	@echo 'K_LINKER $(K_LINKER)'
	@echo 'B_OBJS $(B_OBJS)'
	@echo 'B_ASMFILES $(B_ASMFILES)'
	@echo 'CFLAGS $(CFLAGS)'
	@echo 'CWARNS $(CWARNS)'

QEMUDRVS = \
	-drive format=raw,file=$(MELONOS),index=0,media=disk \
	-drive format=raw,file=$(MELONFS),index=1,media=disk

qemu-boot:
	$(QEMU) -drive format=raw,file=$(BOOT)

qemu:
	$(QEMU) \
		$(QEMUDRVS) \
		-d 'int,cpu_reset,guest_errors,in_asm,exec' \
		-no-reboot -D .qemu.log \
		-serial file:.uart.log \
		-monitor stdio

qemu-ncurse:
	$(QEMU) \
		$(QEMUDRVS) \
		-d 'int,cpu_reset,guest_errors,in_asm,exec' \
		-no-reboot -D .qemu.log \
		-serial file:.uart.log \
		-display curses \
		-monitor stdio

qemu-debug-ncurse:
	$(QEMU) \
		$(QEMUDRVS) \
		-s -S \
		-no-reboot \
		-serial file:.uart.log \
		-display curses

qemu-debug:
	$(QEMU) \
		$(QEMUDRVS) \
		-s -S \
		-no-reboot \
		-serial file:.uart.log

elf-headers:
	readelf -headers $(KERNEL)

d:
	objdump -d $(KERNEL)

hex:
	hexdump -C $(MELONOS)

watch:
	tail -f -n 1 .uart.log

cc:
	bear -- make $(MELONOS)

# subfolder makefiles
include boot/Makefile
include kernel/Makefile
include lib/Makefile
include mkfs/Makefile


-include .local.mk
