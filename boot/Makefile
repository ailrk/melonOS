B_CFILES		= $(wildcard $(B_DIR)/*.c)
B_ASMFILES		= $(B_DIR)/bootld.s
B_LINKER		= $(B_DIR)/boot.ld
B_OBJS			= $(B_CFILES:.c=.o) $(B_ASMFILES:.s=.o)
B_INCS 			= -I$(L_DIR) -Ii386
B_LIBS			= -L. -lmelon

$(BOOT): $(B_OBJS) $(LIBMELON)
	@echo "boot> Building Melon OS Boot loader..."
	$(CC) -T $(B_LINKER) -o $@ $(CFLAGS) $(CWARNS) $(B_OBJS) -lgcc $(B_LIBS)

$(B_CFILES:.c=.o): %.o: %.c
	@echo "boot> compiling $<..."
	$(CC) $(B_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c

$(B_ASMFILES:.s=.o): %.o: %.s
	@echo "boot> compiling $<... "
	@$(AS) $^ -o $@
