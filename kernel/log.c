#include <stdarg.h>
#include "log.h"
#include "vga.h"
#include "debug.h"


void log(char *fmt, ...) {
    va_list args;
    va_start (args, fmt);
    log_vprintf(fmt, args);
    va_end (args);
}

void log_vprintf(char *fmt, va_list args) {
    vga_vprintf(fmt, args);
#ifdef DEBUG
    debug_vprintf(fmt, args);
#endif
}
