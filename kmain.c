#include "tty.h"
#include "idt.h"

// sample C function.
// We need to establish a stack from the bool sector to run this function.
int kmain(void) {
    load_idt();

    return 0;
}
