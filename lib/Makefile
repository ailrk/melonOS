L_CFILES		= $(wildcard $(L_DIR)/*.c)
L_OBJS			= $(L_CFILES:.c=.o)

$(LIBMELON): $(L_OBJS)
	@echo "Building libmelon"
	$(AR) $@ $^
