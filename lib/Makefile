L_CFILES		= $(wildcard $(L_DIR)/*.c)
L_OBJS			= $(L_CFILES:.c=.o)

$(LIBUTILS): $(L_OBJS)
	@$(AR) $@ $^
