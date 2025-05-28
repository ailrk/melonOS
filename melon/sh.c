#include "sys.h"


int main() {
    for(;;) {
        write(1, "sh1\n", 4);
        write(1, "sh2\n", 4);
        write(1, "sh3\n", 4);
        write(1, "sh4\n", 4);
        write(1, "sh5\n", 4);
    }
    return 0;
}
