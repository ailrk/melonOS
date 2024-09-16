#include "err.h"
#include "i386.h"
#include "tty.h"


__attribute__((noreturn))
void panic(const char *msg) {
    tty_printf("[\033[31mPANIC\033[0m] %s", msg);
    cli();
    for(;;);
}


void perror(const char *msg) {
    tty_printf("[\033[31mERROR\033[0m] %s", msg);
}
