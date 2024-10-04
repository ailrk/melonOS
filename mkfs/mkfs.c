#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "defs.h"
#include "fs/fdefs.h"

/* Make melonfs file image */


int fd;
char buf[BSIZE];

void wsec(off_t sec, char *buf) {
    if (lseek(fd, sec * BSIZE, 0) != sec * BSIZE) {
        perror("lseek");
        exit(1);
    }
    if (write(fd, buf, BSIZE) != BSIZE) {
        perror("write");
        exit(1);
    }
}


void rsec(off_t sec, char *buf) {
    if (lseek(fd, sec * BSIZE, 0) != sec * BSIZE) {
        perror("lseek");
        exit(1);
    }

    if (read(fd, buf, BSIZE) != BSIZE) {
        perror("read");
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    char *img = argv[1];
    fd = open(img, O_RDWR | O_CREAT, 00666);

    SuperBlock sb = (SuperBlock) {
        .nblocks   = 30,
        .ninodes   = 5,
        .ndata     = 20,
        .bmapstart = 6,
        .datastart = 7,
    };

    // sb
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &sb, sizeof(SuperBlock));
    wsec(0, buf);

    // inodes
    memset(buf, 0, sizeof(buf));
    wsec(1, buf);
    wsec(2, buf);
    wsec(3, buf);
    wsec(4, buf);
    wsec(5, buf);

    // bmap
    wsec(6, buf);

    // data
    for (unsigned i = 8; i <= sb.nblocks; ++i) {
        wsec(i, buf);
    }

    return 240;
}
