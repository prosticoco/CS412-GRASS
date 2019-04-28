#include "cmd.h"
#include "error.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#define NUM_COMMANDS 12

command_t cmds[NUM_COMMANDS] = {
    {"login",1,false,cmd_login},
    {"pass",1,false,cmd_pass},
    {"ping",1,false,cmd_ping},
    {"date",0,true,cmd_date},
    {"whoami",0,true,cmd_whoami},
    {"w",0,true, cmd_w},
    {"logout",0,true,cmd_logout},
    {"exit",0,false, cmd_exit},
    {"ls", 0, true, cmd_ls},
    {"mkdir", 1, true, cmd_mkdir}, 
    {"cd", 1, true, cmd_cd},
    {"rm",1,true, cmd_rm}
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
    std::cout << "Processing cmd" << std::endl;
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

    char cmd[MAX_INPUT_SIZE];
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
    bzero(out, MAX_INPUT_SIZE);
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
    curr_co->curr_node = curr_co->root_node;
    strncpy(curr_co->pwd, curr_co->root, MAX_PATH_SIZE);
    strncpy(curr_co->curr_out, "User is logged out", MAX_ARG_SIZE);
    return 0;
}

int cmd_exit(connection_t* curr_co) {
    curr_co->exit = true;
    strncpy(curr_co->curr_out, "", MAX_ARG_SIZE);
    return 0;
}

int cmd_ls(connection_t* curr_co) {
    printf("--- ls ---\n");
    char cmd[MAX_INPUT_SIZE] = "ls -l ";
    strncat(cmd, curr_co->pwd, MAX_PATH_SIZE);
    char out[MAX_INPUT_SIZE];
    bzero(out, MAX_INPUT_SIZE);
    execute_system_cmd(cmd, out, MAX_INPUT_SIZE);
    printf("Output : %s\n", out);
    strncpy(curr_co->curr_out, out, MAX_INPUT_SIZE);
    return 0;
}

int cmd_mkdir(connection_t* curr_co) {
    printf("--- mkdir ---\n");
    char cmd[MAX_INPUT_SIZE] = "mkdir ";
    char dir_path[MAX_PATH_SIZE];
    bzero(dir_path, MAX_PATH_SIZE);
    strncat(dir_path, curr_co->pwd, MAX_PATH_SIZE);
    
    char splitted_path[MAX_TOKENS_PATH][MAX_FOLDER_NAME_SIZE];
    char in[MAX_PATH_SIZE];
    bzero(in, MAX_PATH_SIZE);
    strncpy(in, curr_co->pwd, MAX_PATH_SIZE);
    int tokens = tokenize_path(in, splitted_path);
    strcat(dir_path, "/" );
    strncat(dir_path, curr_co->curr_args[0], MAX_PATH_SIZE);
    strcat(cmd, dir_path);

    char out[MAX_INPUT_SIZE];
    bzero(out,MAX_INPUT_SIZE);
    printf("Mkdir command : %s\n", cmd);
    execute_system_cmd(cmd, out, MAX_INPUT_SIZE);
    if (strlen(out) > 0) {
        //fail
        strcpy(curr_co->curr_out, "Mkdir failed to create directory");
    } else {
        strcpy(curr_co->curr_out, "Creation successful");
        Node* child = new Node();
        child->setParent(curr_co->curr_node);
        child->setFolderName(curr_co->curr_args[0]);

        curr_co->root_node->addChildAt(child, splitted_path[tokens-1]);

    }

    return 0;
}

int cmd_cd(connection_t* curr_co) {
    printf("--- cd ---\n");
    if(strlen(curr_co->curr_args[0]) > MAX_PATH_SIZE) {
        return ERROR_MAX_PATH_SIZE;
    }
    
    const int base_length = strlen(curr_co->root);
    const int FULL_PATH_SIZE = MAX_PATH_SIZE + base_length;
    char nwd[FULL_PATH_SIZE];
    bzero(nwd, FULL_PATH_SIZE);

    char new_path[FULL_PATH_SIZE];
    bzero(new_path, FULL_PATH_SIZE );
    
    // check if path is valid
    char in[FULL_PATH_SIZE];
    char splitted_path[MAX_TOKENS_PATH][MAX_FOLDER_NAME_SIZE];

    bzero(in, FULL_PATH_SIZE);
    strncpy(in, curr_co->curr_args[0], FULL_PATH_SIZE);
    int tokens = tokenize_path(in, splitted_path);

    if(strncmp(curr_co->curr_args[0], "/", 1 )==0) {
        strncpy(new_path, curr_co->root, FULL_PATH_SIZE);
        curr_co->curr_node = curr_co->root_node;
        if(tokens == 0) {
            curr_co->curr_node = curr_co->root_node;
            strncpy(curr_co->pwd, new_path, FULL_PATH_SIZE);
            char out[MAX_INPUT_SIZE];
            bzero(out, MAX_INPUT_SIZE);
            strcpy(out, "pwd : /");
            strncpy(curr_co->curr_out, out, MAX_OUTPUT_SIZE);
            return 0;
        }
    } else {
        strncpy(new_path, curr_co->pwd, FULL_PATH_SIZE);
    }
    int i = 0;
    Node * curr_no = curr_co->curr_node;
    
    while(i < tokens) {
        if(strncmp(splitted_path[i], ".",strlen(splitted_path[i])) == 0) {
            i++;
            continue;
        }
        if(strncmp(splitted_path[i], "..",strlen(splitted_path[i])) == 0) {
            curr_no = curr_no->getParent();
            if(curr_no == NULL) {
                return ERROR_ACCESS_DENIED;
            }
            if(strcmp(curr_no->getFolderName(), ROOT) != 0) {
                strcat(new_path, "/");
                strncat(new_path, curr_no->getFolderName(), MAX_FOLDER_NAME_SIZE);
            }
            i++;
            continue;
        }
        
        curr_no = curr_no->checkName(splitted_path[i]);
        if(curr_no == NULL) {
            return ERROR_INVALID_PATH;
        }

        strcat(new_path, "/");
        strncat(new_path, curr_no->getFolderName(), MAX_FOLDER_NAME_SIZE);
    
        i++;
    }

    curr_co->curr_node = curr_no;
 
    char buf[FULL_PATH_SIZE];
    bzero(buf, FULL_PATH_SIZE);
    // rewrite new path
    
    while(strncmp(curr_no->getFolderName(), ROOT,MAX_FOLDER_NAME_SIZE) != 0) {
        char tmp[FULL_PATH_SIZE];
        strncpy(tmp, buf, FULL_PATH_SIZE);
        if(strncmp(curr_no->getFolderName(), ROOT,MAX_FOLDER_NAME_SIZE) != 0) {
            strncpy(buf, curr_no-> getFolderName(), MAX_FOLDER_NAME_SIZE);
            strcat(buf, "/");
            strncat(buf, tmp, MAX_FOLDER_NAME_SIZE);
        }
        curr_no = curr_no->getParent();
    }
    
    strncpy(new_path,curr_co->root, FULL_PATH_SIZE);
    
    strcat(new_path, "/");   
    strncat(new_path, buf, FULL_PATH_SIZE);
    printf("cd to %s\n", new_path);
    strncpy(curr_co->pwd, new_path, FULL_PATH_SIZE);
    char out[MAX_INPUT_SIZE];
    bzero(out, MAX_INPUT_SIZE);
    strcpy(out, "pwd : /");
    strncat(out, buf, FULL_PATH_SIZE);
    strncpy(curr_co->curr_out, out, FULL_PATH_SIZE);
    return 0;
}

int cmd_rm(connection_t* curr_co) {
    if(strlen(curr_co->curr_args[0]) > MAX_FOLDER_NAME_SIZE) {
        return ERROR_FOLDER_NAME_SIZE;
    }
    char cmd[MAX_INPUT_SIZE];
    bzero(cmd, MAX_INPUT_SIZE);
    strcpy(cmd, "rm -r ");
    strncat(cmd, curr_co->pwd, MAX_PATH_SIZE);
    strcat(cmd, "/");
    strncat(cmd, curr_co->curr_args[0], MAX_FOLDER_NAME_SIZE);
    char out[MAX_OUTPUT_SIZE];
    bzero(out, MAX_OUTPUT_SIZE);
    execute_system_cmd(cmd, out, MAX_INPUT_SIZE);
    if(strlen(out) > 0) {
        //failure
        strcpy(curr_co->curr_out, "Command rm failed");
    } else {
        //success
        strcpy(curr_co->curr_out, "Removed ");
        strncat(curr_co->curr_out, curr_co->curr_args[0], MAX_FOLDER_NAME_SIZE);
    }
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

int tokenize_path(char* path, char (*out)[MAX_FOLDER_NAME_SIZE]) {
    int i = 0;
    int token_num = 0;
    // on recoit le premier token
    char* token = strtok(path, "/");
    while(token != NULL){
        token_num ++;
        strncpy(out[i],token,MAX_ARG_SIZE);
        token = strtok(NULL,"/");      
        i += 1;
    }
    return token_num;
}