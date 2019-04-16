#ifndef CMD_H
#define CMD_H

#include "grass.h"

#define MAX_TOKENS 5
#define MAX_INPUT_SIZE 256

int process_cmd(connection_t* curr_co);

int tokenize_cmd(char* cmd,char (*out)[MAX_ARG_SIZE]);

int cmd_login(connection_t* curr_co);

int cmd_pass(connection_t* curr_co);

int cmd_ping(connection_t* curr_co);

int cmd_date(connection_t* curr_co);

int cmd_whoami(connection_t* curr_co);

int cmd_w(connection_t* curr_co);

int cmd_logout(connection_t* curr_co);

int cmd_exit(connection_t* curr_co);

#endif