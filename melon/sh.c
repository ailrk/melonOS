#include "sys.h"

/* melonshell */


#define NARGS 32

typedef enum CmdType {
    EXEC,
    PIPE,
} CmdType;


/* Subtyping with tagged struct. */
typedef struct Cmd {
    CmdType type;
} Cmd;


typedef struct ExecCmd {
    CmdType type;
    char *args[NARGS];
} ExecCmd;


typedef struct PipeCmd {
    CmdType type;
    Cmd *from_cmd;
    Cmd *to_cmd;
} PipeCmd;


int parse_cmd();
Cmd *get_cmd();
void exec_cmd(ExecCmd *cmd);
void pipe_cmd(PipeCmd *pipe);


int main() {
    write(1, "sh", 3);
    return 0;
}
