#include "driver/ide.h"
#include "sys.h"
#include "melon.h"
#include "string.h"
#include "sys/types.h"

/* melonshell
 *
 * cmd    := seq
 * seq    := pipe (';' seq)?
 * pipe   := exec ('|' pipe)?
 * exec   := WORD*
 * */


#define NARGS 2
#define BUF_SZ 512


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



static const char whitespace[] = " \t\r\n";
static const char symbols[]    = "|&;()";


char     linebuffer[BUF_SZ];
char    *p;                 // line buffer pointer


/* We use sbrk as a bump allocator. Heap memory is allocated for
 * commands, when process a new line we free all memory for the
 * previous command at once.
 * */
unsigned allocated = 0;     // total number of allocated bytes


void *alloc(int n) { // allocate n bytes
    allocated += n;
    return sbrk(n);
}


void free() { // free all allocated heap memory
    sbrk(-allocated);
    allocated = 0;
}


void clear() { // clear all state and memory.
    memset(linebuffer, -1, BUF_SZ);
    p = linebuffer;
    free();
}


int is_eof() {
    return linebuffer[0];
}


void skip_whitespaces() {
    while (*p && strchr(whitespace, *p)) p++;
}


/* Move pointer to the end of the word and mark it as the end with '\0'.
 * This works assume the line buffer is still valid.
 * */
char *parse_word() {
    char *begin;
    skip_whitespaces();
    begin = p;
    while (*p && !strchr(whitespace, *p) && !strchr(symbols, *p)) {
        p++;
    }
    *p = '\0';
    return begin;
}


Cmd *parse_exec() {
    ExecCmd *cmd = alloc(sizeof(ExecCmd));
    cmd->type    = EXEC;
    char *arg    = 0;
    int   i      = 0;
    while (!strchr(symbols, *p)) {
        if (i > NARGS) {
            return 0;
        }
        arg = parse_word();
        cmd->argv[i++] = arg;
        skip_whitespaces();
    }
    return (Cmd *) cmd;
}


Cmd *parse_pipe() {
    Cmd *cmd = parse_exec();
    skip_whitespaces();
    if (*p == '|') {
        p++;
        PipeCmd *pcmd = alloc(sizeof(PipeCmd));
        pcmd->type = PIPE;
        pcmd->cmd1 = cmd;
        pcmd->cmd2 = parse_pipe();
        return (Cmd *) pcmd;
    }
    return cmd;
}


Cmd *parse_seq() {
    Cmd *cmd = parse_pipe();
    skip_whitespaces();
    if (*p == ';') {
        p++;
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
    Cmd  *cmd = parse_seq();
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


int next_line() {
    clear();
    printf("> ");

    int i = 0;
    while (i < BUF_SZ - 1) {
        char c;
        int n = read(0, &c, 1);  // read from stdin

        if (n <= 0) break;       // EOF or error
        if (c == '\n') break;    // end of line

        linebuffer[i++] = c;
    }

    linebuffer[i] = '\0';  // null-terminate
    return i > 0;
}


int main() {
    int pid = 0;
    Cmd *cmd = 0;

    while (next_line()) {
        printf("[DEBUG] cmd \n");
        cmd = parse_cmd();
        if (cmd == 0) {
            printf("failed to parse the command\n");
            continue;
        }

        pid = fork();

        if (pid == 0) {
            run_cmd(cmd);
        }
        wait();
    }
    return 0;
}
