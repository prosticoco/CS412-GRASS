#include "cmd.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>


/**
 * @brief Processes command
 * 
 * @param cmd the raw command as input to be processed 
 * @param out the output of the command to be sent back to the client
 * @return int 0 if successful, < 0 if error 
 */
int process_cmd(char* cmd,char* out){
    if(cmd == NULL || strlen(cmd) > MAX_INPUT_SIZE){
        return ERROR_NULL;
    }
    char splitted_cmd[MAX_TOKENS][MAX_ARG_SIZE];
    size_t num_tokens = tokenize_cmd(cmd,splitted_cmd);

    int i = 0;
    int err = 0;
    bool found = false;
    while(i < NUM_COMMANDS && !found) {
        if(strncmp(splitted_cmd[0], cmds[i].name, MAX_ARG_SIZE) == 0) {
            found = true;
            err = cmds[i].fct(&splitted_cmd[1]); 
        }
        i++;
    }

    if(!found) return ERROR_CMD;

    return err;

}


int cmd_login(char (*args)[MAX_ARG_SIZE]){
    printf("Logging in \n");
    return 0;
}

int cmd_pass(char (*args)[MAX_ARG_SIZE]){
    printf("THANKS FOR THE PASSWORD \n");
    return 0;
}

int tokenize_cmd(char *in, char (*out)[MAX_ARG_SIZE] ){
    int i = 0;
    int token_num = 0;
    // on recoit le premier token
    char* token = strtok(in, " ");
    // on met la condition i < 4 dans la boucle afin de garantir de ne pas tokeniser plus de 4 entrées
    while(token != NULL && i < MAX_TOKENS){
        token_num ++;

        token = strtok(NULL," ");
        i += 1;
    }
    return token_num;
}