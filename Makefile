CC= i686-elf-gcc
AS = nasm -f elf32
CFLAGS = -ffreestanding -g -nostdlib
CWARNS =-Wall -Wextra -fno-exceptions
CFILES   = $(wildcard *.c)
ASMFILES = $(wildcard *.s)
OBJFILES = $(CFILES:.c=.o) $(ASMFILES:.s=.o)
OUT =melonos.img
QEMU =qemu-system-i386
DOSBOX =dosbox

$(OUT): $(OBJFILES)
	@echo "> linking object files... "
	@$(CC) -T linker.ld -o $@ $(CFLAGS) $(CWARNS) $^ -lgcc

%.o: %.s
	@echo "> compiling .s files... "
	@$(AS) $< -o $@

%.o: %.c
	@echo "> compiling .o files... "
	@$(CC) $(CFLAGS) $(CWARNS) -c -o $@ $<


.PHONY: clean run
clean:
	rm -rf *.o $(OUT)

qemu:
	$(QEMU) -drive format=raw,file=$(OUT) -device virtio-vga,xres=640,yres=320

qemu-debug:
	$(QEMU) -drive format=raw,file=$(OUT) -d 'int,cpu_reset,guest_errors,in_asm,exec' -D .qemu.log -monitor stdio

qemu-debug-int:
	$(QEMU) -drive format=raw,file=$(OUT) -d 'int' -D .qemu.log -monitor stdio
