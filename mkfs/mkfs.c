#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>

/* Make melonfs file image */


int fd;

int main(int argc, char *argv[]) {
    char *img = argv[1];
    fd = open(img, O_RDWR | O_CREAT, 00666);
    return 0;
}
