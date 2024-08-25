#include "tty.h"
#include "idt.h"

// sample C function.
// We need to establish a stack from the bool sector to run this function.
int kmain(void) {
    load_idt();

    vga_tty_init();
    vga_tty_write_string("HI");

    return 0;
}
