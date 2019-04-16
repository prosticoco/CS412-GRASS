#include "cmd.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#define NUM_COMMANDS 4

command_t cmds[NUM_COMMANDS] = {
    {"login",1,false,cmd_login},
    {"pass",1,false,cmd_pass},
    {"ping",1,false,cmd_ping},
    {"w",0, true, cmd_w}
};

/**
 * @brief Processes command
 * 
 * @param cmd the raw command as input to be processed 
 * @param out the output of the command to be sent back to the client
 * @return int 0 if successful, < 0 if error 
 */
int process_cmd(connection_t * curr_co){
    if(curr_co->curr_in == NULL || strlen(curr_co->curr_in) > MAX_INPUT_SIZE){
        printf("Error null command or too long \n");
        return ERROR_NULL;
    }
    curr_co->curr_in[strlen(curr_co->curr_in)-1] = '\0';
    char splitted_cmd[MAX_TOKENS][MAX_ARG_SIZE];
    int num_tokens = tokenize_cmd(curr_co->curr_in,splitted_cmd);
    if(num_tokens <= 0){
        printf("Error : Tokenizer : %d \n",num_tokens);
        return ERROR_TOKEN;
    }
    int i = 0;
    int err = 0;
    bool found = false;
    while(i < NUM_COMMANDS && !found) {
        if(strncmp(splitted_cmd[0], cmds[i].name, MAX_ARG_SIZE) == 0) {
            found = true;
            //todo initialize data structure to pass to cmd function
            curr_co->curr_args = &splitted_cmd[1];

            //todo check auth if needed
            if( cmds[i].authent) {
                if(curr_co->auth) {
                    err = cmds[i].fct(curr_co); 
                } else {
                    strncpy(curr_co->curr_out,"Authentication required \n", MAX_ARG_SIZE);
                }
            } else {
                err = cmds[i].fct(curr_co); 
            }
        }
        i++;
    }

    if(!found){
        strcpy(curr_co->curr_out,"Please provide a valid command \n");
        return err;

    } 

    return err;

}


int cmd_login(connection_t* curr_co){
    printf("Logging in \n");
    curr_co->ready_for_check = true;
    strncpy(curr_co->username,curr_co->curr_args[0],MAX_USERNAME_SIZE);
    strcpy(curr_co->curr_out,"Please provide your password \n");
    return 0;
}

int cmd_pass(connection_t * curr_co){
    if(curr_co->auth) {
        strcpy(curr_co->curr_out,"User already authentified \n");
        return 0;
    }
    if(!(curr_co->ready_for_check)){
        strcpy(curr_co->curr_out,"Please provide a username first \n");
        return 0;
    }    
    bool found = false;
    for(auto& user : curr_co->server_data->users) {
        if( strncmp(user->uname, curr_co->username, strlen(user->uname)) == 0) {
            if(strncmp(curr_co->curr_args[0], user->pass,  strlen(user->pass))== 0) {
                curr_co->auth = true;
                strncpy(curr_co->curr_out,"Authentication successful \n", MAX_ARG_SIZE);
                found = true;
                break;
            }
        }
        
    }
    if(!found) {
        strcpy(curr_co->curr_out,"Invalid credentials \n");     
    }
    curr_co->ready_for_check = false;
    return 0;
}

int cmd_w(connection_t* curr_co) {

    // there is always at least one user authentified
    std::cout << "Command W" << std::endl;
    for (auto co : curr_co->server_data->connections) {
        if (co->auth) {
            std::cout << "User authentified : " << co->username << std::endl;        
            strncat(curr_co->curr_out, co->username, MAX_ARG_SIZE);
            strcat(curr_co->curr_out, " ");
        }
    }
    return 0;
}

int cmd_ping(connection_t* curr_co) {
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
        strncpy(out[i],token,MAX_ARG_SIZE);
        token = strtok(NULL," ");      
        i += 1;
    }
    return token_num;
}