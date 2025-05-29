MELIB_CFILES  	= $(wildcard $(MELIB_DIR)/*.c)
MELIB_OBJS    	= $(MELIB_CFILES:.c=.o) $(MELIB_ASM_OBJS)

MELIB_ASMS      = $(MELIB_DIR)/syscalls.s
MELIB_ASM_PPS   = $(MELIB_DIR)/syscalls.pp.s
MELIB_ASM_OBJS  = $(MELIB_DIR)/syscalls.o

MELIB_LINKER  	= $(M_DIR)/user.ld
MELIB_CFLAGS  	= $(CFLAGS) $(CWARNS) $(MELIB_INCS)
MELIB_INCS    	= -Ii386 -I$(L_DIR) -I$(K_DIR)

# -I$(shell find kernel -type d -exec echo -I"{}" \;)


$(LIBMELON): $(MELIB_OBJS)
	$(AR) $@ $^


# Step 1: Preprocess .s to .pp.s
$(MELIB_ASM_PPS): $(MELIB_ASMS)
	$(CPP) $(MELIB_INCS) $< | tools/pptrim > $@


# Step 2: Assemble .pp.s to .o
$(MELIB_ASM_OBJS): $(MELIB_ASM_PPS)
	$(AS) $< -o $@


# Step 3. Compile library objects
$(MELIB_DIR)/%.o: $(MELIB_DIR)/%.c
	$(CC) $(MELIB_CFLAGS) -c $< -o $@
