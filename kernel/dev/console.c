#include "dev/console.h"
#include "driver/vga.h"
#include "fs/dev.h"

void init_console() {
    vga_init();
    devices[DEV_CONSOLE].read = 0x0;
    devices[DEV_CONSOLE].write = 0x0;
}
