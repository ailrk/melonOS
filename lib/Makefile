LIB_CFILES	= $(wildcard $(LIB_DIR)/*.c)
LIB_OBJS	= $(LIB_CFILES:.c=.o)

$(LIBUTILS): $(LIB_OBJS)
	$(AR) $@ $^

CFLAGS += -fPIC

# Compile directly from source using HOSTCC
lib_test_so:
	$(HOSTCC) -shared -fPIC -o libutils.so $(LIB_CFILES) -I$(LIB_DIR) -DHOST_TEST
