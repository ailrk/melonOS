#pragma once


#include "fdefs.fwd.h"
typedef char (*console_getc_t)();

void console_init();
void console_handler(console_getc_t);
