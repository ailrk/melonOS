BOOT_CFILES 	= $(wildcard $(BOOT_DIR)/*.c)
BOOT_ASMFILES 	= $(BOOT_DIR)/bootld.s
BOOT_LINKER 	= $(BOOT_DIR)/boot.ld
BOOT_OBJS 		= $(BOOT_CFILES:.c=.o) $(BOOT_ASMFILES:.s=.o)
BOOT_INCS 		= -I$(LIB_DIR) -Ii386
BOOT_LIBS 		= -L. -lutils

$(BOOT): $(BOOT_OBJS) $(LIBUTILS)
	$(LD) -T $(BOOT_LINKER) -o $@ $(BOOT_OBJS) $(BOOT_LIBS)

$(BOOT_CFILES:.c=.o): %.o: %.c
	$(CC) $(BOOT_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c

$(BOOT_ASMFILES:.s=.o): %.o: %.s
	$(AS) $^ -o $@
