#include "melon.h"
#include "string.h"

/* The shell
 *
 * cmd    := seq
 * seq    := pipe (';' seq)?
 * pipe   := exec ('|' pipe)?
 * exec   := WORD*
 * */


#define NARGS 3
#define BUF_SZ 512

const char whitespace[] = " \t\r\n";
const char symbols[]    = "|&;()";


typedef enum CmdType {
    EXEC,
    PIPE,
    SEQ,
} CmdType;


/* Subtyping with tagged struct. */
typedef struct Cmd {
    CmdType type;
} Cmd;


typedef struct ExecCmd {
    CmdType  type;
    char    *argv[NARGS];
} ExecCmd;


typedef struct SeqCmd {
    CmdType  type;
    Cmd     *cmd1;
    Cmd     *cmd2;
} SeqCmd;


typedef struct PipeCmd {
    CmdType type;
    Cmd    *cmd1;
    Cmd    *cmd2;
} PipeCmd;


/*
 * Gloal State of the shell.
 * */
struct State {
    char     buf[BUF_SZ];
    char    *p;            // line buffer pointer

    /* We use sbrk as a bump allocator. Heap memory is allocated for
     * commands, when process a new line we free all memory for the
     * previous command at once.
     * */
    unsigned allocated; // total number of allocated bytes
} st = {
    .buf       = { 0 },
    .p         = 0,
    .allocated = 0
};


/* Allocate n bytes */
void *alloc(int n) {
    st.allocated += n;
    return sbrk(n);
}


/* Free all allocated heap memory */
void free() {
    if (st.allocated > 0) {
        sbrk(-st.allocated);
    }

    st.allocated = 0;
}


/* clear all state and memory. */
void clear() {
    memset(st.buf, 0, BUF_SZ);
    st.p = st.buf;
    free();
}


int is_eof() {
    return st.buf[0];
}


void skip_whitespaces() {
    while (*st.p && strchr(whitespace, *st.p)) st.p++;
}


/* Move pointer to the end of the word and mark it as the end with '\0'.
 * This works assume the line buffer is still valid.
 * */
char *parse_word() {
    char *begin;
    skip_whitespaces();
    begin = st.p;
    while (*st.p && !strchr(whitespace, *st.p) && !strchr(symbols, *st.p)) {
        st.p++;
    }
    *st.p = '\0';
    return begin;
}


/* Parse a simple executable command */
Cmd *parse_exec() {
    ExecCmd *cmd = alloc(sizeof(ExecCmd));
    cmd->type    = EXEC;
    char *arg    = 0;
    int   i      = 0;
    while (*st.p && !strchr(symbols, *st.p)) {
        if (i > NARGS) {
            return 0;
        }
        arg = parse_word();
        st.p++;
        cmd->argv[i++] = arg;
        skip_whitespaces();
    }
    return (Cmd *) cmd;
}


/* Parse a pipeline */
Cmd *parse_pipe() {
    Cmd *cmd = parse_exec();
    skip_whitespaces();
    if (*st.p == '|') {
        st.p++;
        PipeCmd *pcmd = alloc(sizeof(PipeCmd));
        pcmd->type = PIPE;
        pcmd->cmd1 = cmd;
        pcmd->cmd2 = parse_pipe();
        return (Cmd *) pcmd;
    }
    return cmd;
}


/* Parse a sequence */
Cmd *parse_seq() {
    Cmd *cmd = parse_pipe();
    skip_whitespaces();
    if (*st.p == ';') {
        st.p++;
        SeqCmd *scmd = alloc(sizeof(SeqCmd));
        scmd->type = SEQ;
        scmd->cmd1 = cmd;
        scmd->cmd2 = parse_seq();
        return (Cmd *) scmd;
    }
    return cmd;
}


Cmd *parse_cmd() {
    skip_whitespaces();
    Cmd *cmd = parse_seq();
    return cmd;
}


void run_cmd(Cmd *cmd) {
    switch (cmd->type) {
        case EXEC:
            break;
        case PIPE:
            break;
        case SEQ:
            break;
    }
}


/* Read a line from buffer */
char *gets() {
  char gets_buf[512];
  int  pos = 0;
  int  len = 0;

  int i = 0;
  char c;

  while (i < BUF_SZ - 1) {
      if (pos >= len) {
          len = read(0, gets_buf, sizeof(gets_buf));
          pos = 0;
          if (len <= 0) break; // EOF
      }

      c = gets_buf[pos++];
      st.buf[i++] = c;
      if (c == '\n' || c == '\r') break;
  }

  st.buf[i] = '\0';

  if (i == 0 && len <= 0) {
      return 0;
  }
  return st.buf;
}


int next_line() {
    printf("> ");
    clear();
    gets();
    if (st.buf[0] == 0)  return 0; // EOF
    return 1;
}

#if DEBUG && DEBUG_SH
void print_indent(unsigned indent) {
    for (int i = 0; i < indent; ++i) {
        printf(" ");
    }
}

void dump_cmd(Cmd *cmd, unsigned indent) {
    // Print indents
    print_indent(indent);
    printf("| ");

    switch (cmd->type) {
        case EXEC:
            for (int i = 0; i < NARGS; ++i) {
                char *arg = ((ExecCmd *)cmd)->argv[i];
                if (arg) {
                    printf("%s ", arg);
                }
            }
            break;
        case PIPE:
            printf("pipe> \n");
            dump_cmd(((PipeCmd *)cmd)->cmd1, indent + 1);
            dump_cmd(((PipeCmd *)cmd)->cmd2, indent + 1);
            break;
        case SEQ:
            printf("seq> \n");
            dump_cmd(((PipeCmd *)cmd)->cmd1, indent + 1);
            dump_cmd(((PipeCmd *)cmd)->cmd2, indent + 1);
            break;
    }
    printf("\n");
}
#endif


int main() {
    int pid = 0;
    Cmd *cmd = 0;

    while (next_line()) {
        cmd = parse_cmd();

#if DEBUG && DEBUG_SH
        dump_cmd(cmd, 0);
#endif

        if (cmd == 0) {
            printf("failed to parse the command\n");
            continue;
        }

        pid = fork();

        if (pid == 0) {
            run_cmd(cmd);
        } else {
            wait();
        }
    }
    return 0;
}
