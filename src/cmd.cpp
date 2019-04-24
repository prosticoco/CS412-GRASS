#include "cmd.h"
#include "error.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#define NUM_COMMANDS 8

command_t cmds[NUM_COMMANDS] = {
    {"login",1,false,cmd_login},
    {"pass",1,false,cmd_pass},
    {"ping",1,false,cmd_ping},
    {"date",0,true,cmd_date},
    {"whoami",0,true,cmd_whoami},
    {"w",0,true, cmd_w},
    {"logout",0,true,cmd_logout},
    {"exit",0,false, cmd_exit}
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
            // check if args have correct length

            std::cout << "Tokens : " << num_tokens << std::endl;
            std::cout << "params required : " << cmds[i].num_params << std::endl;
            if (num_tokens - 1 != cmds[i].num_params) {
                strncpy(curr_co->curr_out,"Invalid number of arguments", MAX_ARG_SIZE);
                return err;
            }
            //check auth if needed
            if( cmds[i].authent) {
                if(curr_co->auth) {
                    err = cmds[i].fct(curr_co); 
                } else {
                    strncpy(curr_co->curr_out,"Authentication required", MAX_ARG_SIZE);
                }
            } else {
                err = cmds[i].fct(curr_co); 
            }
        }
        i++;
    }

    if(!found){
        strcpy(curr_co->curr_out,"Please provide a valid command");
        return err;

    } 

    return err;

}


int cmd_login(connection_t* curr_co){
    printf("Logging in \n");
    curr_co->ready_for_check = true;
    strncpy(curr_co->username,curr_co->curr_args[0],MAX_USERNAME_SIZE);
    strcpy(curr_co->curr_out,"Please provide your password");
    return 0;
}

int cmd_pass(connection_t * curr_co){
    if(curr_co->auth) {
        strncpy(curr_co->curr_out,"User already authentified", MAX_ARG_SIZE);
        return 0;
    }
    if(!(curr_co->ready_for_check)){
        strncpy(curr_co->curr_out,"Please provide a username first", MAX_ARG_SIZE);
        return 0;
    }    
    bool found = false;
    for(auto& user : curr_co->server_data->users) {
        if( strncmp(user->uname, curr_co->username, strlen(user->uname)) == 0) {
            if(strncmp(curr_co->curr_args[0], user->pass,  strlen(user->pass))== 0) {
                curr_co->auth = true;
                strncpy(curr_co->curr_out,"Authentication successful", MAX_ARG_SIZE);
                found = true;
                break;
            }
        }
        
    }
    if(!found) {
        strcpy(curr_co->curr_out,"Invalid credentials");     
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

    char cmd[MAX_INPUT_SIZE+1];
    bzero(cmd,sizeof(cmd));
    sprintf(cmd, "ping %s -c 1", curr_co->curr_args[0]);
    printf("%s\n", curr_co->curr_args[0]);
    char out[MAX_INPUT_SIZE];
    bzero(out,sizeof(out));
    execute_system_cmd(cmd, out, MAX_INPUT_SIZE);
    strncpy(curr_co->curr_out, out, MAX_INPUT_SIZE);

    return 0;
}

int cmd_date(connection_t* curr_co) {
    char out[MAX_INPUT_SIZE];
    execute_system_cmd("date", out, MAX_INPUT_SIZE);
    strncpy(curr_co->curr_out, out, MAX_INPUT_SIZE);
    return 0;
}

int cmd_whoami(connection_t* curr_co) {
    strncpy(curr_co->curr_out, curr_co->username, MAX_USERNAME_SIZE + 1);
    return 0;
}

int cmd_logout(connection_t* curr_co) {
    curr_co->auth = false;
    strncpy(curr_co->curr_out, "User is logged out", MAX_ARG_SIZE);
    return 0;
}

int cmd_exit(connection_t* curr_co) {
    curr_co->exit = true;
    strncpy(curr_co->curr_out, "", MAX_ARG_SIZE);
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