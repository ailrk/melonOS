#include "string.h"
#include "melon.h"


#define O_RDONLY 0

struct dirent {
    unsigned int inum;
    char name[14];
};

struct stat {
    short type;
    int dev;
    unsigned int ino;
    short nlink;
    unsigned int size;
};

#define T_DIR  1
#define T_FILE 2

// void ls(char *path) {
//     int fd;
//     struct dirent de;
//     struct stat st;
//     char buf[128];
//     char *p;

//     if ((fd = open(path, O_RDONLY)) < 0) {
//         printf("ls: cannot open %s\n", path);
//         return;
//     }

//     /* Read dirents one at a time */
//     while (read(fd, (char *)&de, sizeof(de)) == sizeof(de)) {
//         if (de.inum == 0)
//             continue;

//         /* Build full path for stat */
//         int plen = 0;
//         while (path[plen]) plen++;

//         int i = 0;
//         while (i < plen && i < 127) { buf[i] = path[i]; i++; }
//         if (i < 127 && buf[i-1] != '/') buf[i++] = '/';
//         int j = 0;
//         while (de.name[j] && j < 14 && i < 127) buf[i++] = de.name[j++];
//         buf[i] = '\0';

//         /* Print name + type */
//         printf("%s\n", de.name);
//     }

//     close(fd);
// }


// int main(int argc, char *argv[]) {
//     if (argc < 2)
//         ls(".");
//     else {
//         int i;
//         for (i = 1; i < argc; i++)
//             ls(argv[i]);
//     }
//     exit();
// }

int main(int argc, char *argv[]) {
}
