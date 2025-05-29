# We use c preprocessor to process both nasm assembly files and linker files.
# because they are not supported by gcc, we need to process them manually with
# `cpp`.
# A preprocessed file will have `.pp` extension being added before it's suffix.
# For example, linker.ld will become linker.pp.ld, and the linker.pp.ld will be
# used in the following linking process.
KERN_CFILES 	= $(shell find $(KERN_DIR) -name "*.c")
KERN_VECTORS 	= $(KERN_DIR)/trap/vectors.s # vectors.s is generated from vectors.py
KERN_ASMFILES 	= $(filter-out $(KERN_VECTORS) $(shell find $(KERN_DIR) -name "*.pp.s"),$(shell find $(KERN_DIR) -name "*.s")) $(KERN_VECTORS)
KERN_LINKER 	= $(KERN_DIR)/kernel.ld
KERN_OBJS 	= $(KERN_CFILES:.c=.o) $(KERN_ASMFILES:.s=.o)
KERN_INCS 	= $(shell find kernel -type d -exec echo -I"{}" \;) -I$(LIB_DIR) -Ii386
KERN_LIBS 	= -L. -lutils


$(KERNEL): $(KERN_OBJS) $(LIBUTILS)
	@echo "kernel> Building Melon OS Kernel..."
	@echo "$(KERN_ASMFILES)"
	@echo "$(KERN_ASMFILES)"
	@echo "$(KERN_OBJS)"
	@echo "$(KERN_INCS)"
	$(CPP) $(KERN_INCS) $(KERN_LINKER) | tools/pptrim > $(KERN_LINKER:.ld=.pp.ld)
	$(CC) -T $(KERN_LINKER:.ld=.pp.ld) $(CFLAGS) $(CWARNS) $(KERN_OBJS) -o $@ $(KERN_LIBS)

$(KERN_CFILES:.c=.o): %.o: %.c
	$(CC) $(KERN_INCS) $(CFLAGS) $(CWARNS) -c -o $@ $*.c

$(KERN_ASMFILES:.s=.o): %.o: %.s
	$(CPP) $(KERN_INCS) $^ | tools/pptrim > $(^:.s=.pp.s)
	$(AS) $(^:.s=.pp.s) -o $@

$(KERN_VECTORS): $(KERN_VECTORS:.s=.py)
	rm -f $(KERN_VECTORS) $(KERN_VECTORS:.s=.pp.s)
	$(KERN_DIR)/trap/vectors.py > $@
