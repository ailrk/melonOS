MELON_CFILES 	= $(wildcard $(MELON_DIR)/*.c)
MELON_OBJS 		= $(MELON_CFILES:.c=.o)
MELON_LINKER 	= $(MELON_DIR)/user.ld
MELON_LIBS 		= -L. -lutils -lmelon
MELON_INCS 		= -I$(KERN_DIR) -Ii386 -I$(MELON_DIR) -I$(MELIB_DIR)


USERPROGS = $(M_CFILES:.c=_)


$(USERPROGS): %_: %.o $(LIBUTILS) $(LIBMELON)
	$(CC) -T $(M_LINKER) $(CFLAGS) $(CWARNS) -Wl,-emain $^ -o $@ $(M_LIBS)


$(M_CFILES:.c=.o): %.o: %.c
	$(CC) $(M_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c
