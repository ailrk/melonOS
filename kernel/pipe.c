#include <stdint.h>
#include <string.h>
#include "pipe.h"
#include "fdefs.fwd.h"
#include "file.h"
#include "palloc.h"
#include "process.h"
#include "spinlock.h"
#include "stdlib.h"
#include "err.h"

/* Pipe is a buffer that is shared by two files. We call one file read end and
 * the other write end. If you write into it through one file, you can read
 * it from the other file.
 * */

#define PIPESZ 512

typedef struct Pipe {
    char     data[PIPESZ]; // ring buffer.
    SpinLock lk;
    uint32_t nread;
    uint32_t nwritten;
    bool     isreadopen;
    bool     iswriteopen;
} Pipe;



int pipe_allocate(File **read, File **write) {
    Pipe *pipe = 0;
    *read = *write = 0;
    if ((*read = file_allocate()) == 0) {
        goto bad;
    }

    if ((*write = file_allocate()) == 0) {
        goto bad;
    }

    if ((pipe = (Pipe *)palloc()) == 0) {
        goto bad;
    }

    pipe->isreadopen = 1;
    pipe->iswriteopen = 1;
    pipe->nread = 0;
    pipe->nwritten = 0;
    pipe->lk = new_lock("pipe");

    // read end
    (*read)->type = FD_PIPE;
    (*read)->readable = 1;
    (*read)->writable = 0;
    (*read)->pipe = pipe;

    // write end
    (*write)->type = FD_PIPE;
    (*write)->readable = 0;
    (*write)->writable = 1;
    (*write)->pipe = pipe;

    return 0;

bad:
    if (pipe) {
        pfree((char *)pipe);
    }

    if (*read) {
        file_close(*read);
    }

    if (*write) {
        file_close(*write);
    }

    return -1;
}


void pipe_close(Pipe *pipe, bool close_write_end) {
    lock(&pipe->lk); // make sure noone is reading/writing while closing the pipe.
    if (close_write_end) {
        pipe->iswriteopen = 0;
        wakeup(&pipe->nread);
    } else {
        pipe->isreadopen = 0;
        wakeup(&pipe->nwritten);
    }
    unlock(&pipe->lk);

    if (!pipe->isreadopen && !pipe->iswriteopen) {
        pfree((void *)pipe);
    }
}


int pipe_read(Pipe *pipe, char *buf, size_t n) {
    kassert(pipe->nread <= pipe->nwritten);

    int r = 0;
    lock(&pipe->lk);
    // nothing to read, waiting for new data.
    while (pipe->nread == pipe->nwritten && pipe->iswriteopen) {
        if (this_proc()->killed) {
            unlock(&pipe->lk);
            return -1;
        }
        sleep(&pipe->nread, &pipe->lk);
    }

    if (pipe->nread < pipe->nwritten) {
        r = min(pipe->nwritten - pipe->nread, n);
        memcpy(buf, &pipe->data[pipe->nread % PIPESZ], r);
    }

    while (n--) {
        if (pipe->nread == pipe->nwritten) break;
        buf[r++] = pipe->data[pipe->nread++ % PIPESZ];
    }

    wakeup(&pipe->nwritten);
    unlock(&pipe->lk);
    return r;
}


int pipe_write(Pipe *pipe, const char *buf, size_t n) {
    kassert(pipe->nread <= pipe->nwritten);
    int w = 0;
    lock(&pipe->lk);

    while (n--) {
        // wait for a full pipe to be read
        while (pipe->nwritten == pipe->nread + PIPESZ) {
            if (!pipe->isreadopen || this_proc()->killed) {
                unlock(&pipe->lk);
                return -1;
            }
            // give control to read end
            wakeup(&pipe->nread);
            sleep(&pipe->nwritten, &pipe->lk);
        }
        pipe->data[pipe->nwritten++ % PIPESZ] = buf[w++];
    }
    wakeup(&pipe->nread);
    unlock(&pipe->lk);
    return w;
}
