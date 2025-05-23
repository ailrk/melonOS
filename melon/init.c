#include "sys.h"

int main() {
    if (open("console", O_RDWR) < 0) {
        mknod("console", 1, 1);
        open("console", O_RDWR);
    }

    write(1, "hello\n", 6);
}
