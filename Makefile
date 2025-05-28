ARCH ?= i686
CC = $(ARCH)-elf-gcc
LD = $(ARCH)-elf-ld
AS = nasm -f elf32
AR = ar rcs
CPP = cpp
HOSTCC = gcc

#############################
# Parameters
#############################
# DEBUG flag
DEBUG ?= 1

NOGRAPHICS ?= 1

# Enable GDB remote debug. If it's set, make qemu will try to attach
# to a gdb session on localhost:1234
GDB ?= 0

#############################
# Variables
#############################

# CFLAGS
CFLAGS = -ffreestanding -nostdlib -fno-omit-frame-pointer
ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
endif


CWARNS = -Wall -Wextra -fno-exceptions


# Directories
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

echo-kernel-elf:
	@echo "$(KERNEL)"

echo-bootloader:
	@echo "$(BOOTLOADER)"

echo-img:
	@echo "$(MELONOS)"

# order of the import matters here.
include melon/Makefile
userprogs: $(USERPROGS)


$(MELONOS): $(BOOT) $(KERNEL) $(MELONFS)
	dd if=/dev/zero of=$(MELONOS) count=10000
	dd if=$(BOOT) of=$(MELONOS) conv=notrunc
	dd if=$(KERNEL) of=$(MELONOS) seek=20 conv=notrunc

$(MELONFS): $(MKFS) $(USERPROGS)
	./$(MKFS) $(MELONFS) $(USERPROGS)


.PHONY: clean qemu-debug copy echo
clean:
	find $(K_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	find $(B_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	find $(L_DIR) \( -name "*.o" -o -name "*.pp.*" \) -exec rm {} \;
	find $(M_DIR) \( -name "*.o" -o -name "*.pp.*" -o -name "*_" \) -exec rm {} \;
	rm -rf *.o *.pp.* $(MELONOS) $(MELONFS) $(MKFS) $(BOOT) $(KERNEL) $(LIBUTILS) $(LIBMELON)

echo:
	@echo 'DEBUG $(DEBUG)'
	@echo 'NOGRAPHICS $(NOGRAPHICS)'
	@echo 'GDB $(GDB)'
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
	@echo 'USERPROGS $(USERPROGS)'


#############################
# QEMU settings
#############################

# QEMU drives. To simply the development we put the kernel and the
# filesystem in two different drives.
QEMU_DRVS = \
	-drive format=raw,file=$(MELONOS),index=0,media=disk \
	-drive format=raw,file=$(MELONFS),index=1,media=disk

# The file for uart debug output
QEMU_DEBUG_SERIALFILE = .debug.log

# The file for qemu logs
QEMU_LOGFILE = .qemu.log

# QEMU graphics options. Turned off by default.
QEMU_GRAPHICS_FLAGS =

ifeq ($(NOGRAPHICS), 1)
	QEMU_GRAPHICS_FLAGS += -nographic
endif

QEMU_GDB_FLAGS =

ifeq ($(GDB), 1)
QEMU_GDB_FLAGS += -s -S
endif


qemu-boot:
	$(QEMU) -drive format=raw,file=$(BOOT)


qemu:
	tools/bridge & \
	$(QEMU) \
		$(QEMU_DRVS) \
		$(QEMU_GDB_FLAGS) \
		-d 'int,cpu_reset,guest_errors,in_asm,exec' \
		-no-reboot -D $(QEMU_LOGFILE) \
		-serial unix:/tmp/qemu-serial.sock,server,nowait \
		-serial file:$(QEMU_DEBUG_SERIALFILE) \
		-monitor stdio \
		-m 512M \
		$(QEMU_GRAPHICS_FLAGS)


print-trace:
	@echo "Monitoring errors..." >&2
	cat $(QEMU_DEBUG_SERIALFILE) | \
		sed -n '/trapframe>\|Stack trace:\|PANIC\|ERROR/,$$p' | \
		tools/addr2line-filter $(KERNEL)


debug:
	tail -f $(QEMU_DEBUG_SERIALFILE) | \
		tools/addr2line-filter $(KERNEL)


elf-headers:
	readelf -headers $(KERNEL)

d:
	objdump -d $(KERNEL)

hex:
	hexdump -C $(MELONOS)

serial:
	tools/serial

cc:
	bear -- make $(MELONOS)

# subfolder makefiles
include boot/Makefile
include kernel/Makefile
include lib/Makefile
include mkfs/Makefile

-include .local.mk
