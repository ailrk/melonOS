#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "defs.h"
#include "fs/fdefs.fwd.h"
#include "fs/fdefs.h"

/* Make melonfs file image */


int fd, ffd;
char buf[BSIZE];
void wsec (off_t sec, char *buf);
void rsec (off_t sec, char *buf);


int main (int argc, char *argv[]) {
    char *img = argv[1];
    fd = open (img, O_RDWR | O_CREAT, 00666);

    SuperBlock sb = (SuperBlock) {
        .nblocks    = 100,
        .ninodes    = 30,
        .inodestart = 1,
        .ndata      = 70,
        .bmapstart  = 8,
        .datastart  = 9,
    };

    DInode root = (DInode) {
        .type  = F_DIR,
    };

    // sb
    memset (buf, 0, sizeof(buf));
    memcpy (buf, &sb, sizeof(SuperBlock));
    wsec (0, buf);

    // inodes
    memmove (buf, &root, sizeof(DInode));
    wsec (1, buf);
    memset (buf, 0, sizeof(buf));
    wsec (2, buf);
    wsec (3, buf);
    wsec (4, buf);
    wsec (5, buf);

    // bmap
    wsec (6, buf);

    // data
    for (unsigned i = 7; i <= sb.nblocks; ++i) {
        wsec (i, buf);
    }

    // write user program to disk
    for (int i = 2; i < argc; ++i) {
        char *path = argv[i];

        if ((ffd = open (path, O_RDONLY)) < 0) {
            perror (argv[i]);
            exit (1);
        }

        char buf[65];
        memset(buf, 0, sizeof(buf));

        char *p = path + strlen(path) + 1;
        while (*p != '/' && p != path) p--;
        p += 1;
        if (strlen(p) >= 64) {
            fprintf(stderr, "%s\n", p);
            fprintf(stderr, "program name is too long. needs to be less than 64 bytes");
        }

        if (p[strlen(p) - 1] != '_') {
            fprintf(stderr, "%s\n", p);
            fprintf(stderr, "program name must end with _");
        }

        strncpy(buf, p, strlen(p) - 1); // skip following '_'
    }

    return 0;
}


void wsec (off_t sec, char *buf) {
    if (lseek (fd, sec * BSIZE, 0) != sec * BSIZE) {
        perror ("lseek");
        exit (1);
    }
    if (write (fd, buf, BSIZE) != BSIZE) {
        perror ("write");
        exit (1);
    }
}


void rsec (off_t sec, char *buf) {
    if (lseek (fd, sec * BSIZE, 0) != sec * BSIZE) {
        perror ("lseek");
        exit (1);
    }

    if (read (fd, buf, BSIZE) != BSIZE) {
        perror ("read");
        exit (1);
    }
}

