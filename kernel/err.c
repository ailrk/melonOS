#include "err.h"
#include "i386.h"
#include "debug.h"
#include "driver/vga.h"


__attribute__((noreturn))
void panic (const char *msg) {
    vga_printf ("[\033[31mPANIC\033[0m] %s", msg);
    debug_printf ("[PANIC] %s", msg);
    cli ();
    for (;;);
    __builtin_unreachable ();
}


void perror (const char *msg) {
    vga_printf ("[\033[31mERROR\033[0m] %s", msg);
}
