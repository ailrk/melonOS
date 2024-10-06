#include "fs/fdefs.h"
#include "dev/console.h"

Dev devices[NDEV];


void dev_init() {
    console_init();
}
