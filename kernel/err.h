#pragma once


void panic(const char *);
void perror(const char *);


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define kassert(expr)                                         \
    do {                                                      \
        if (!(expr)) {                                        \
            panic("assert: " #expr ","                        \
                    __FILE__ ":" TOSTRING(__LINE__) "\n" );   \
        }                                                     \
    } while (0)
