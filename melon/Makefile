MELON_CFILES 	= $(wildcard $(MELON_DIR)/*.c)
MELON_OBJS 	= $(MELON_CFILES:.c=.o)
MELON_LINKER 	= $(MELON_DIR)/user.ld
MELON_LIBS 	= -L. -lutils -lmelon
MELON_INCS 	= -I$(KERN_DIR) -Ii386 -I$(MELON_DIR) -I$(MELIB_DIR)

USERPROGS 	= $(MELON_CFILES:.c=_)


$(USERPROGS): %_: %.o $(LIBUTILS) $(LIBMELON)
	$(CC) -T $(MELON_LINKER) $(CFLAGS) $(CWARNS) -Wl,-emain $^ -o $@ $(MELON_LIBS)


$(MELON_CFILES:.c=.o): %.o: %.c
	$(CC) $(MELON_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c
