#include "sys.h"

int main() {
    int pid;
    if (open("/console", O_RDWR) < 0) {
        mknod("/console", 1, 1);
        open("/console", O_RDWR);
    }
    dup(0); // stdout
    dup(0); // stderr

    for (;;) {
        write(1, "init\n", 5);
        pid = fork();
        if (pid < 0) {
            write(1, "init: fork\n", 12);
            exit();
        }
        if (pid == 0) {
            write(1, "child\n", 5);
        }
        for (;;) {
            write(1, "parent\n", 7);
        }
    }
}
