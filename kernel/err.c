#include "err.h"
#include "i386.h"
#include "debug.h"
#include "driver/vga.h"


/* Only works for standard cdecl style calling convention.
 * This relies on the structure of
 *
 * [saved EBP]
 * [return address]
 * */
void print_stack_trace() {
    uintptr_t *ebp;
    __asm__ volatile ("mov %%ebp, %0" : "=r"(ebp));

    vga_printf("Stack trace:\n");
    debug_printf("Stack trace:\n");

    for (int i = 0; ebp && i < 15; ++i) {
        uintptr_t ret = ebp[1];
        vga_printf("  [<%08x>]\n", ret);
        debug_printf("  [<%08x>]\n", ret);
        ebp = (uintptr_t*) ebp[0];
    }
}


__attribute__((noreturn))
void panic (const char *msg) {
    vga_printf ("[\033[31mPANIC\033[0m] %s", msg);
    debug_printf ("[\033[31mPANIC\033[0m] %s", msg);
    print_stack_trace();
    cli ();
    for (;;);
    __builtin_unreachable ();
}


void perror (const char *msg) {
    vga_printf ("[\033[31mERROR\033[0m] %s", msg);
    debug_printf ("[\033[31mERROR\033[0m] %s", msg);
}
