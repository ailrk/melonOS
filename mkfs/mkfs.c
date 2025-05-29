#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include "defs.h"
#include "fcntl.h"
#include "fs/fdefs.fwd.h"
#include "fs/fdefs.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

/* Make melonfs file image */

void      bwrite(off_t sec, char *buf);
void      bread(off_t sec, char *buf);
inodeno_t ialloc(unsigned type);
void      iread(inodeno_t ino, DInode *node);
void      iwrite(inodeno_t ino, DInode *node);
void      iappend(inodeno_t ino, void *p, size_t n);
void      balloc(unsigned usedblks);
void      report(const char *fmt, ...);


/* Block buffer  */
char      zeros[BSIZE];

/* File system image */
int       imgfd;

/* The root inode number */
inodeno_t rootino;

/* The next available free inode number */
inodeno_t freeino = 1;

/* The next available block number */
blockno_t freeblock;


/* The super block */
SuperBlock sb;

/* Configure the super block.
 *
 * On disk block structures:
 * [ super | inode .. | freemap .. | data .. ]
 *
 * */
void sbconfig() {
    unsigned nbitmap = NBLKS / (BSIZE * 8) + 1;
    unsigned ninodeblks = NINODES / inode_per_block + 1;
    unsigned nmeta = 1 + ninodeblks + nbitmap;
    unsigned ndata = NBLKS - nmeta;

    assert(ndata > 0);

    // superblock config
    sb = (SuperBlock) {
        .nblocks    = NBLKS,
        .ninodes    = NINODES,
        .inodestart = 1,
        .ndata      = ndata,
        .bmapstart  = 1 + ninodeblks,
        .datastart  = nmeta,
    };

    freeblock = nmeta;
}


void parsename(char *path, char *namebuf, size_t n) {
    memset(namebuf, 0, n);
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

    strncpy(namebuf, p, strlen(p) - 1); // skip following '_'
}

/* Write super block */
void wsb () {
    char buf[BSIZE];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &sb, sizeof(sb));
    bwrite(0, buf);
}


int main(int argc, char *argv[]) {
    DirEntry   de;
    char      *img = argv[1];
    imgfd = open(img, O_RDWR | O_CREAT | O_TRUNC, 00666);

    report("Making %s ...\n", img);

    sbconfig();

    report("====================\n");
    report("Superblock: \n");
    report("  nblocks:    %d\n", sb.nblocks);
    report("  ninodes:    %d\n", sb.ninodes);
    report("  ndata:      %d\n", sb.ndata);
    report("  inodestart: %d\n", sb.inodestart);
    report("  bmapstart:  %d\n", sb.bmapstart);
    report("  datastart:  %d\n", sb.datastart);
    report("====================\n");

    // allocate and clear the entire image.
    for (unsigned i = 0; i <= sb.nblocks; ++i) bwrite(i, zeros);
    report("Allocated %d blocks. \n", sb.nblocks);

    // write superblock
    wsb();

    // allocate root inode
    inodeno_t rootino = ialloc(F_DIR);
    assert(rootino == ROOTINO);

    report("Writing root directory ...\n");

    // point current folder . to root itself
    memset(&de, 0, sizeof(de));
    de.inum = rootino;
    strncpy(de.name, ".", 2);
    iappend(rootino, &de, sizeof(DirEntry));

    // point parent folder .. to root itself
    memset(&de, 0, sizeof(de));
    strncpy(de.name, "..", 3);
    iappend(rootino, &de, sizeof(DirEntry));


    // write user program to disk
    for (int i = 2; i < argc; ++i) {
        char *path = argv[i];
        char  namebuf[DIRNAMESZ];
        int   ffd;

        parsename(path, namebuf, sizeof(namebuf));
        report("writing /%s from %s ...\n", namebuf, path);

        if ((ffd = open (path, O_RDONLY)) < 0) {
            perror (argv[i]);
            exit (1);
        }

        inodeno_t ino = ialloc(T_FILE);

        // add the file's dir entry to the root dir.
        memset(&de, 0, sizeof(de));
        de.inum = ino;
        strncpy(de.name, namebuf, sizeof(namebuf));
        iappend(rootino, &de, sizeof(de));

        // write file content
        unsigned r;
        char buf[BSIZE];
        while ((r = read(ffd, buf, sizeof(buf))) > 0)
            iappend(ino, buf, r);

        close(ffd);
    }

    balloc(freeblock);
    return 0;
}


/* Write content in buf into a a sector. The buffer needs to be BSIZE large */
void bwrite(off_t sec, char *buf) {
    if (lseek(imgfd, sec * BSIZE, SEEK_SET) != sec * BSIZE) {
        perror("lseek");
        exit(1);
    }
    if (write (imgfd, buf, BSIZE) != BSIZE) {
        perror ("write");
        exit (1);
    }
}


/* Read a sector into buf. The buffer needs to be BSIZE large */
void bread(off_t sec, char *buf) {
    if (lseek(imgfd, sec * BSIZE, SEEK_SET) != sec * BSIZE) {
        perror("lseek");
        exit(1);
    }

    if (read(imgfd, buf, BSIZE) != BSIZE) {
        perror("read");
        exit(1);
    }
}


/* Allocate an inode with given inode type */
inodeno_t ialloc(unsigned type) {
    inodeno_t inum = freeino++;
    DInode inode = {
        .type = type,
        .nlink = 1,
        .size = 0,
    };

    iwrite(inum, &inode);
    return inum;
}


/* Read an inode with inode number ino. */
void iread(inodeno_t ino, DInode *node) {
    char buf[BSIZE] = {0};
    blockno_t bn = get_inode_block(ino, &sb);

    bread(bn, buf);
    DInode *p = (DInode *)buf + (ino % inode_per_block);
    *node = *p;
}


/* Write an inode with inode number ino. */
void iwrite(inodeno_t ino, DInode *node) {
    char buf[BSIZE] = {0};
    blockno_t bn = get_inode_block(ino, &sb);

    bread(bn, buf);
    DInode *p = (DInode *)buf + (ino % inode_per_block);
    *p = *node;
    bwrite(bn, buf);
}


/* Append data to inode data section */
void iappend(inodeno_t ino, void *p, size_t n) {
    DInode node;

    // file end
    unsigned off;

    // the nth block that the file end resides in.
    unsigned nth;

    // the block to write
    blockno_t fb;

    // buffer for indirect1 block
    blockno_t indirect1[BSIZE] = {0};

    // content buffer
    char buf[BSIZE] = {0};

    iread(ino, &node);

    off = node.size;

    while (n > 0) {

        nth = off / BSIZE;

        if (nth < NDIRECT) {
            if (node.addrs[nth] == 0) {
                node.addrs[nth] = freeblock++;
            }
            fb = node.addrs[nth];
        } else if (nth < NDIRECT + BSIZE) {
            // NINDIRECT1
            if (node.addrs[NDIRECT] == 0) {
                node.addrs[NDIRECT] = freeblock++;
            }
            bread(node.addrs[NDIRECT], (char *)indirect1);
            if (indirect1[nth - NDIRECT] == 0) {
                indirect1[nth - NDIRECT] = freeblock++;
                bwrite(node.addrs[NDIRECT], (char *)indirect1);
            }
            fb = indirect1[nth - NDIRECT];
        } else {
            fprintf(stderr, "file is too big\n");
            exit(1);
        }

        // always read less than or equal to a sector.
        unsigned realn = min(n, (nth + 1) * BSIZE - off);
        bread(fb, buf);
        memcpy(buf + (off - (nth * BSIZE)), p, realn);
        bwrite(fb, buf);
        n -= realn;
        off += realn;
        p += realn;
    }

    // update size
    node.size = off;
    iwrite(ino, &node);
}


/* Allocate `used` blocks on the filesystem */
void balloc(unsigned usedblks) {
    char buf[BSIZE] = {0};
    report("allocated %d blocks\n", usedblks);
    memset(buf, 0, sizeof(buf));
    for (unsigned i = 0; i < usedblks; ++i) {
        buf[i/8] = buf[i/8] | (0x1 << (i % 8));
    }
    bwrite(sb.bmapstart, buf);
}


void report(const char *fmt, ...) {
    va_list args;
    va_start (args, fmt);
    printf("\033[32m[mkfs]\033[0m ");
    vprintf(fmt, args);
    va_end (args);
}
