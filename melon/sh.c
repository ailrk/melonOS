#include "sys.h"
#include "melon.h"
#include "string.h"

/* melonshell */


#define NARGS 2

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


#define BUF_SZ 512

/* line buffer */
char linebuffer[BUF_SZ];

/* cmd buffer */
char cmdbuffer[BUF_SZ];


int  parse_cmd();
int  run_cmd();
void exec_cmd(ExecCmd *cmd);
void pipe_cmd(PipeCmd *pipe);


int is_eof() {
    return linebuffer[0];
}


int next_line() {
    printf("> ");
    memset(linebuffer, 0, BUF_SZ);
    memset(cmdbuffer, 0, BUF_SZ);
    read(1, linebuffer, BUF_SZ);
    return is_eof();
}


int parse_cmd() {

}


int main() {
    int pid = 0;

    while (next_line()) {
        if (parse_cmd() == -1) {
            printf("failed to parse the command\n");
            continue;
        }

        pid = fork();

        if (pid == 0) {
            run_cmd();
        }
        wait();
    }
    return 0;
}
