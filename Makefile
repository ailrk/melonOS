CC				= i686-elf-gcc
AS				= nasm -f elf32
CPP				= cpp
CFLAGS			= -ffreestanding -g -nostdlib
CWARNS			= -Wall -Wextra -fno-exceptions

# We use c preprocessor to process both nasm assembly files and linker files. 
# because they are not supported by gcc, we need to process them manually with
# `cpp`. 
# A preprocessed file will have `.pp` extension being added before it's suffix.
# For example, linker.ld will become linker.pp.ld, and the  linker.pp.ld will be
# used in the following linking process.
K_CFILES		= $(filter-out boot.c,$(wildcard *.c))
K_ASMFILES		= $(filter-out bootld.s $(wildcard *.pp.s),$(wildcard *.s))
K_LINKER		= kernel.ld
K_OBJS			= $(K_CFILES:.c=.o) $(K_ASMFILES:.s=.o)

B_CFILES		= boot.c ata.c tty.c string.c
B_ASMFILES		= bootld.s
B_LINKER		= boot.ld
B_OBJS			= $(B_CFILES:.c=.o) $(B_ASMFILES:.s=.o)

BOOT			= melonos-bootloader
KERNEL			= melonos-kernel
OUT				= melonos.img

QEMU			= qemu-system-i386


.PHONY: boot kernel out

default: out

out: $(OUT)

boot: $(BOOT)

kernel: $(KERNEL)

$(OUT): $(BOOT) $(KERNEL)
	dd if=/dev/zero of=$(OUT) count=10000
	dd if=$(BOOT) of=$(OUT) conv=notrunc
	dd if=$(KERNEL) of=$(OUT) seek=80 conv=notrunc

$(BOOT): $(B_OBJS)
	@echo "Building Melon OS Boot loader..."
	$(CC) -T $(B_LINKER) -o $@ $(CFLAGS) $(CWARNS) $(B_OBJS) -lgcc

$(KERNEL): $(K_OBJS)
	@echo "Building Melon OS Kernel..."
	@$(CPP) $(K_LINKER) | tools/pptrim > $(K_LINKER:.ld=.pp.ld)
	$(CC) -T $(K_LINKER:.ld=.pp.ld) -o $@ $(CFLAGS) $(CWARNS) $(K_OBJS) -lgcc

%.o: %.c
	@echo "> compiling $<..."
	$(CC) $(CFLAGS) $(CWARNS) -c -o $@ $^

%.o: %.s
	@echo "> preprocessing $<... "
	@$(CPP) $^ | tools/pptrim > $(^:.s=.pp.s)
	@echo "> compiling $(^:.s=.pp.s) (from $^)... "
	$(AS) $(^:.s=.pp.s) -o $@


.PHONY: clean qemu-debug copy echo
clean:
	rm -rf *.o *.pp.* $(OUT) $(BOOT) $(KERNEL)

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
	@echo 'CFLAGS	  $(CFLAGS)'
	@echo 'CWARNS	  $(CWARNS)'

qemu:
	$(QEMU) -drive format=raw,file=$(OUT) -device virtio-vga,xres=640,yres=320

qemu-debug:
	$(QEMU) -drive format=raw,file=$(OUT) -d 'int,cpu_reset,guest_errors,in_asm,exec' -no-reboot -D .qemu.log -monitor stdio
