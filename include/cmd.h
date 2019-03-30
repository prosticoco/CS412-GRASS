#include "grass.h"

#define NUM_COMMANDS 2
#define MAX_TOKENS 2
#define MAX_ARG_SIZE 32
#define MAX_INPUT_SIZE 256

int process_cmd(char* cmd,size_t buffer_size);

int tokenize_cmd(char* cmd,char** output);

int cmd_login(char** args);

int cmd_pass(char** args);


command_t cmds[NUM_COMMANDS] = {
    {"login",1,false,cmd_login},
    {"pass",1,false,cmd_pass}
};