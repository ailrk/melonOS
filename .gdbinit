target remote localhost:1234
# b test1
b trap if tf->trapno == (0x20 + 0xe)
