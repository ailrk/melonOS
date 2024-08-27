#include "kbd.h"


uint32_t kbd_get_status() {
    return inb(KBD_DATA);
}

uint32_t kbd_get_data() {
    return inb(KBD_STATP); 
}
