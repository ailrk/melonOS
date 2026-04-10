#pragma once

#include <stdarg.h>


/* User system call interfaces */

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREAT   0x200

#define T_DIR  1
#define T_FILE 2
#define T_DEV  3


typedef struct Stat {
    short     type;
    int       dev;
    int       inum;
    short     nlink;
    unsigned  size;
} Stat;


typedef struct DirEntry {
    int  inum;
    char name[14];
} DirEntry;


int   fork();
int   exit() __attribute__((noreturn));
int   wait();
int   kill(int pid);
int   exec(char *, char **);
int   getpid();
int   sleep(int);
void *sbrk(int);
int   open(const char *, int);
int   close(int fd);
int   read(int fd, char *buf, int size);
int   write(int fd, const char *buf, int size);
int   mknod(const char *path, short major, short minor);
int   mkdir(const char *path);
int   link(const char *, const char*);
int   unlink(const char *);
int   dup(int fd);
int   pipe(int *);


/* The melonos user library */

void  putc(char c);
void  printf(char *fmt, ...);
void  vprintf(char *fmt, va_list args);
