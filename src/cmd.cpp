#include "cmd.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>
#define NUM_COMMANDS 2

command_t cmds[NUM_COMMANDS] = {
    {"login",1,false,cmd_login},
    {"pass",1,false,cmd_pass}
};

/**
 * @brief Processes command
 * 
 * @param cmd the raw command as input to be processed 
 * @param out the output of the command to be sent back to the client
 * @return int 0 if successful, < 0 if error 
 */
int process_cmd(char* cmd,char* out){
    if(cmd == NULL || strlen(cmd) > MAX_INPUT_SIZE){
        printf("Error null command or too long \n");
        return ERROR_NULL;
    }
    cmd[strlen(cmd)-1] = '\0';
    char splitted_cmd[MAX_TOKENS][MAX_ARG_SIZE];
    int num_tokens = tokenize_cmd(cmd,splitted_cmd);
    if(num_tokens <= 0){
        printf("Error tokenizer \n");
        return ERROR_TOKEN;
    }
    printf("command name : [%s]\n",splitted_cmd[0]);
    printf("real command name : [%s]\n",cmds[0].name);
    int i = 0;
    int err = 0;
    bool found = false;
    while(i < NUM_COMMANDS && !found) {
        if(strncmp(splitted_cmd[0], cmds[i].name, MAX_ARG_SIZE) == 0) {
            printf("found command \n");
            found = true;
            err = cmds[i].fct(&splitted_cmd[1]); 
        }
        i++;
    }

    if(!found){
        printf("Unknown command \n");
        return ERROR_CMD;

    } 

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
    printf("toooooooken time \n");
    int i = 0;
    int token_num = 0;
    // on recoit le premier token
    char* token = strtok(in, " ");
    printf("test1\n");
    // on met la condition i < 4 dans la boucle afin de garantir de ne pas tokeniser plus de 4 entrées
    while(token != NULL && i < MAX_TOKENS){
        printf("test2\n");
        token_num ++;
        strncpy(out[i],token,MAX_ARG_SIZE);
        token = strtok(NULL," ");      
        i += 1;
    }
    printf("test3\n");
    return token_num;
}