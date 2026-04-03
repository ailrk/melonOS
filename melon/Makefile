MELON_CFILES 	= $(wildcard $(MELON_DIR)/*.c)
MELON_OBJS 	= $(MELON_CFILES:.c=.o)
MELON_LINKER 	= $(MELON_DIR)/user.ld
MELON_PP_LINKER = $(MELON_LINKER:.ld=.pp.ld)
MELON_LIBS 	= -L. -lutils -lmelon
MELON_INCS 	= -I$(KERN_DIR) -Ii386 -I$(MELON_DIR) -I$(MELIB_DIR)

USERPROGS 	= $(MELON_CFILES:.c=_)


# Build user programs depend on the custom linker files, that's why
# it's in the dependency. But we also need ot filter it out in the
# command.
$(USERPROGS): %_: %.o $(LIBUTILS) $(LIBMELON) $(MELON_PP_LINKER)
	$(CC) -T $(MELON_PP_LINKER) $(CFLAGS) $(CWARNS) -Wl,-emain $(filter %.o %.a, $^) -o $@ $(MELON_LIBS)

$(MELON_PP_LINKER): $(MELON_LINKER)
	$(CPP) $(MELON_INCS) $< | tools/pptrim > $@

$(MELON_CFILES:.c=.o): %.o: %.c
	$(CC) $(MELON_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c
