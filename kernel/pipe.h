#pragma once

#include <stddef.h>
#include "fdefs.fwd.h"


int  pipe_allocate(File **in, File **out);
void pipe_close(Pipe *pipe, bool close_write_end);
int  pipe_read(Pipe *pipe, char *buffer, size_t n);
int  pipe_write(Pipe *pipe, const char *buffer, size_t n);
