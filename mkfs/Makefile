F_CFILES = $(F_DIR)/mkfs.c
F_INCS = -I$(K_DIR) -Ii386

$(MKFS): $(F_CFILES)
	$(HOSTCC) $(F_INCS) $(CWARNS) -g -o $@ $^
