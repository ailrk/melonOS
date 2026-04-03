#include "melon.h"
#include "fmt.h"
#include "sys.h"
#include <stddef.h>

/* io */
void putc(char c) {
    write(2, &c, 1);
}


static char *putchar(char *c) { putc(*c++); return c; }


char *gets(char *buf, int nmax) {
  char gets_buf[512];
  int  pos = 0;
  int  len = 0;

  int i = 0;
  char c;

  while (i < nmax - 1) {
      if (pos >= len) {
          len = read(0, gets_buf, sizeof(gets_buf));
          pos = 0;
          if (len <= 0) break; // EOF
      }

      c = gets_buf[pos++];
      buf[i++] = c;
      if (c == '\n' || c == '\r') break;
  }

  buf[i] = '\0';

  if (i == 0 && len <= 0) {
      return 0;
  }
  return buf;
}


void printf(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


void vprintf(char *fmt, va_list args) {
    FmtIO io = {
        .putchar = putchar
    };

    format(io, fmt, args);
}
