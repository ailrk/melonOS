LIB_CFILES		= $(wildcard $(LIB_DIR)/*.c)
LIB_OBJS		= $(LIB_CFILES:.c=.o)

$(LIBUTILS): $(LIB_OBJS)
	$(AR) $@ $^
