M_CFILES 	= $(wildcard $(M_DIR)/*.c)
M_OBJS 		= $(M_CFILES:.c=.o)
M_LINKER 	= $(M_DIR)/user.ld
M_LIBS 		= -L. -lutils -lmelon
M_INCS 		= -I$(K_DIR) -Ii386 -I$(M_DIR) -I$(MELIB_DIR)


USERPROGS = $(M_CFILES:.c=_)


$(USERPROGS): %_: %.o $(LIBUTILS) $(LIBMELON)
	$(CC) -T $(M_LINKER) $(CFLAGS) $(CWARNS) -Wl,-emain $^ -o $@ $(M_LIBS)


$(M_CFILES:.c=.o): %.o: %.c
	$(CC) $(M_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c
