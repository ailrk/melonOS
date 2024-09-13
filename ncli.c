#include "ncli.h"
#include "err.h"
#include "i386.h"
#include "proc.h"
#include "tty.h"
#include <stdint.h>


/* n push_cli requires n pop_cli to undo. 
 *
 * If interrupt was disabled before pushing then
 * when ncli is 0 don't do anything.
 * */

void push_cli() {
    uint32_t eflags = readeflags();
    cli();
    if (this_cpu()->ncli == 0) {
        this_cpu()->int_on = eflags & FL_IF;
    }
    this_cpu()->ncli += 1;
}

void pop_cli() {
    if (--this_cpu()->ncli <  0) {
        panic("pop_cli");
    }
    if (!this_cpu()->ncli && this_cpu()->int_on)
        sti();
}
