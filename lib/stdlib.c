#include "stdlib.h"
#include "ctype.h"
#include "errno.h"


int atoi(const char *p) {
    int k   = 0;
    int neg = 0;
    while (isspace(*p)) p++;

    if (*p == '+') p++;
    else if (*p == '-') {
        neg = 1;
        p++;
    }

    while (isdigit(*p)) {
        k = k * 10 + (*p - '0');
        p++;
    }
    if (neg)
        return -k;
    else
        return k;
}


/*! Convert string to long integer
 *  this strtol only support hexdecimal, octal and decimal.
 *  If an error occurs, `strtol` returns a 0 and set `errno`  to ERANGE.
 *  @str:       pointer to the string
 *  @endptr:    points to the character after the last digit. 0 if not needed.
 *
 *  @return:    0 if error
 * */
long int strtol (const char *str, const char **endptr) {
    const char *p = str;
    if (endptr) {
        *endptr = p;
    }
    long int k = 0;
    int neg = 0;

    while (isspace(*p)) p++;

    if (*p == '+') p++;
    else if (*p == '-') {
        neg = 1;
        p++;
    }

    if (p[0] == '0' && p[1] == 'x') { // hex
        p += 2;
        while (isdigit(*p) || *p > 'a' && *p < 'f' || *p > 'A' && *p < 'F') {
            k = k * 16;
            if (isdigit(*p))
                k += *p - '0';
            else if (*p > 'a' && *p < 'f')
                k += *p - 'a' + 10;
            else if (*p > 'A' && *p < 'F')
                k += *p - 'A' + 10;
            else
                goto abort;
            p++;
        }

    } else if (p[0] == '0') { // oct
        p += 1;
        while(isdigit(*p)) {
            k = k * 8 + (*p - '0');
            p++;
        }

    } else if (isdigit(p[0])) { // dec
        while(isdigit(*p)) {
            k = k * 10 + (*p - '0');
            p++;
        }
    }

    if (endptr) *endptr = p;

    if (neg)
        return -k;
    else
        return k;

abort:
    if (endptr) *endptr = p;
    errno = ERANGE;
    return 0;
}
