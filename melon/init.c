#include "fs/fcntl.h"
#include "sys.h"


int main() {
    if (open("console", O_RDWR) < 0) {
        mknod("console", 1, 1);
        open("console", O_RDWR);
    }

    dup(0);
    dup(0);

    write(1, "hello\n", 6);
}
