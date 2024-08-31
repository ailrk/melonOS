#include "err.h"
#include "i386.h"
#include "tty.h"


__attribute__((noreturn))
void panic(const char *msg) {
    vga_tty_printf("[PANIC] %s", msg);
    cli();
    for(;;);
}
