#include "driver/ps2.h"
#include "fs/fdefs.h"
#include "dev/console.h"

Dev devices[NDEV];


void dev_init() {
    ps2_init();
    console_init();
}
