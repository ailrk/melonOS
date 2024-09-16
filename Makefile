ARCH ?= i686
CC = $(ARCH)-elf-gcc
LD = $(ARCH)-elf-ld
AS = nasm -f elf32
AR = ar rcs
CPP = cpp
CFLAGS = -ffreestanding -g -nostdlib
CWARNS = -Wall -Wextra -fno-exceptions

B_DIR = boot
K_DIR = kernel
L_DIR = lib

BOOT = melonos-bootloader
KERNEL = melonos-kernel
LIBMELON = libmelon.a
OUT = melonos.img

QEMU = qemu-system-i386

.PHONY: boot kernel all

default: all
all: $(OUT)
boot: $(BOOT)
kernel: $(KERNEL)

$(OUT): $(BOOT) $(KERNEL)
	dd if=/dev/zero of=$(OUT) count=10000
	dd if=$(BOOT) of=$(OUT) conv=notrunc
	dd if=$(KERNEL) of=$(OUT) seek=20 conv=notrunc


.PHONY: clean qemu-debug copy echo
clean:
	find $(K_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	find $(B_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	find $(L_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	rm -rf *.o *.pp.* $(OUT) $(BOOT) $(KERNEL) $(LIBMELON)

echo:
	@echo 'CC		  $(CC)'
	@echo 'AS		  $(AS)'
	@echo 'OUT		  $(OUT)'
	@echo 'BOOT		  $(BOOT)'
	@echo 'KERNEL     $(KERNEL)'
	@echo 'K_OBJS	  $(K_OBJS)'
	@echo 'K_ASMFILES $(K_ASMFILES)'
	@echo 'K_LINKER   $(K_LINKER)'
	@echo 'B_OBJS	  $(B_OBJS)'
	@echo 'B_ASMFILES $(B_ASMFILES)'
	@echo 'CFLAGS     $(CFLAGS)'
	@echo 'CWARNS	  $(CWARNS)'

qemu-boot:
	$(QEMU) -drive format=raw,file=$(BOOT)

qemu:
	$(QEMU) -drive format=raw,file=$(OUT) -device virtio-vga,xres=640,yres=320

qemu-log:
	$(QEMU) -drive format=raw,file=$(OUT) -d 'int,cpu_reset,guest_errors,in_asm,exec' \
		-no-reboot -D .qemu.log \
		-serial file:.uart.log \
		-monitor stdio

elf-headers:
	readelf -headers $(KERNEL)

d:
	objdump -d $(KERNEL)

hex:
	hexdump -C $(OUT)

watch:
	tail -f -n 1 .uart.log

cc:
	bear -- make $(OUT)

# subfolder makefiles
include boot/Makefile
include kernel/Makefile
include lib/Makefile

-include .local.mk
