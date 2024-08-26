#include "tty.h"
#include "idt.h"
#include "i386.h"

void handler1() {
    vga_tty_write_string("hello from handler\n");
    iret();
}

// sample C function.
// We need to establish a stack from the bool sector to run this function.
int kmain(void) {
    load_idt();
    vga_tty_init();
    regist_idt_handler(0x9, &handler1, InterruptGate);

    int_(0x9);

    vga_tty_write_string("hihihi\n");

    return 0;
}
