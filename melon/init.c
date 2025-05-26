#include "sys.h"

int main() {
    if (open("/console", O_RDWR) < 0) {
        mknod("/console", 1, 1);
        open("/console", O_RDWR);
    }
    dup(0); // stdout
    dup(0); // stderr

    for (;;) {
        write(1, "init\n", 5);
    }
}
