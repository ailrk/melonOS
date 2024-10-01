F_CFILES = $(wildcard $(F_DIR)/*.c)
F_OBJS = $(F_CFILES:.c=.o)
F_INCS = -I$(K_DIR) -Ii386


$(MELONFS): $(F_OBJS)

$(F_CFILES:.c=.o): %.o: %.c
	$(CC) $(F_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c
