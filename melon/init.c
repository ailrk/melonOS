#include "sys.h"


char *argv[] = { "sh", 0 };

int main() {
    int pid;
    int wpid;

    if (open("/console", O_RDWR) < 0) {
        mknod("/console", 1, 1);
        open("/console", O_RDWR);
    }
    dup(0); // stdout
    dup(0); // stderr

    pid = fork();
    if (pid < 0) {
        write(1, "init: fork\n", 12);
        exit();
    }
    if (pid == 0) {
        exec("/sh", argv);
        exit();
    }
    while ((wpid = wait()) >= 0 && pid != wpid) {
        write(1, "zombie\n", 7);
    }
}
