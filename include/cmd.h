#ifndef CMD_H
#define CMD_H

#include "grass.h"

#define MAX_TOKENS 2
#define MAX_INPUT_SIZE 256

int process_cmd(char* cmd,char* out);

int tokenize_cmd(char* cmd,char (*out)[MAX_ARG_SIZE]);

int cmd_login(char (*args)[MAX_ARG_SIZE]);

int cmd_pass(char (*args)[MAX_ARG_SIZE]);

#endif