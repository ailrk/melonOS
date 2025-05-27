M_CFILES = $(wildcard $(M_DIR)/*.c)
M_INCS = -I$(K_DIR) -Ii386 -I$(M_DIR)
M_ASMFILES = $(M_DIR)/syscalls.s
M_OBJS = $(M_CFILES:.c=.o) $(M_ASMFILES:.s=.o)
M_LINKER = $(M_DIR)/user.ld
M_LIBS = -L. -lutils

USERPROGS = $(M_CFILES:.c=_)

$(USERPROGS): %_: %.o $(M_ASMFILES:.s=.o) $(LIBUTILS)
	$(CC) -T $(M_LINKER) $(CFLAGS) $(CWARNS) -Wl,-emain $^ -o $@ $(M_LIBS)

$(M_CFILES:.c=.o): %.o: %.c
	$(CC) $(M_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c

$(M_ASMFILES:.s=.o): %.o: %.s
	$(CPP) $(K_INCS) $^ | tools/pptrim > $(^:.s=.pp.s)
	$(AS) $(^:.s=.pp.s) -o $@
