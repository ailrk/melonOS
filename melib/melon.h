#pragma once

#include <stdarg.h>
#include "fs/fcntl.h"


/* User system call interfaces */

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




