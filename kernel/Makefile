# We use c preprocessor to process both nasm assembly files and linker files.
# because they are not supported by gcc, we need to process them manually with
# `cpp`.
# A preprocessed file will have `.pp` extension being added before it's suffix.
# For example, linker.ld will become linker.pp.ld, and the linker.pp.ld will be
# used in the following linking process.
K_CFILES = $(shell find $(K_DIR) -name "*.c")

# vectors.s is generated from vectors.py
K_VECTORS = $(K_DIR)/vectors.s

K_ASMFILES = $(filter-out $(K_VECTORS) $(shell find $(K_DIR) -name "*.pp.s"),$(shell find $(K_DIR) -name "*.s")) $(K_VECTORS)

K_LINKER = $(K_DIR)/kernel.ld
K_OBJS = $(K_CFILES:.c=.o) $(K_ASMFILES:.s=.o)
K_INCS = $(shell find kernel -type d -exec echo -I"{}" \;) -I$(L_DIR) -Ii386
K_LIBS = -L. -lmelon


$(KERNEL): $(K_OBJS) $(LIBMELON)
	@echo "kernel> Building Melon OS Kernel..."
	@echo "$(K_ASMFILES)"
	@echo "$(K_ASMFILES)"
	@echo "$(K_OBJS)"
	@echo "$(K_INCS)"
	@echo "kernel> Building Melon OS Kernel..."
	@$(CPP) $(K_INCS) $(K_LINKER) | tools/pptrim > $(K_LINKER:.ld=.pp.ld)
	$(CC) -T $(K_LINKER:.ld=.pp.ld) $(CFLAGS) $(CWARNS) $(K_OBJS) -o $@ -lgcc $(K_LIBS)

$(K_CFILES:.c=.o): %.o: %.c
	@echo "kernel> compiling $<..."
	$(CC) $(K_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c

$(K_ASMFILES:.s=.o): %.o: %.s
	@echo "kernel> compiling $<... "
	@$(CPP) $(K_INCS) $^ | tools/pptrim > $(^:.s=.pp.s)
	@$(AS) $(^:.s=.pp.s) -o $@

$(K_VECTORS): $(K_VECTORS:.s=.py)
	rm -f $(K_VECTORS) $(K_VECTORS:.s=.pp.s)
	$(K_DIR)/vectors.py > $@
