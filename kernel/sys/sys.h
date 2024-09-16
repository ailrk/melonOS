#pragma once

/* user system call interfaces */
int   fork();
int   exit() __attribute__((noreturn));
int   wait();
int   kill(int);
int   exec(char*, char**);
int   getpid();
char *sbrk(int);
int   sleep(int);
