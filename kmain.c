#include "tty.h"
#include "idt.h"
#include "i386.h"

// sample C function.
// We need to establish a stack from the bool sector to run this function.
int kmain(void) {
    load_idt();
    vga_tty_init();
    vga_tty_printf("this is int %d %d, this is %p", 123, -123, &load_idt);
    return 0;
}
