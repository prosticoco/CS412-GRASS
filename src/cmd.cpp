#include "cmd.h"
#include "error.h"
#include "utils.h"
#include "ftp.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#define NUM_COMMANDS 15

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
    {"rm",1,true, cmd_rm},
    {"get",1,true,cmd_get},
    {"put",2,true,cmd_put},
    {"grep", 1, true, cmd_grep}
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
    bzero(cmd,MAX_INPUT_SIZE);
    sprintf(cmd, "ping %s -c 1", curr_co->curr_args[0]);
    printf("%s\n", curr_co->curr_args[0]);
    char out[MAX_INPUT_SIZE];
    bzero(out,MAX_INPUT_SIZE);
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

    //testing
    char cd[MAX_PATH_SIZE];
    bzero(cd, MAX_PATH_SIZE);
    char new_path[MAX_PATH_SIZE + MAX_ROOT_PATH + MAX_ARG_SIZE];
    bzero(new_path, MAX_PATH_SIZE + MAX_ROOT_PATH + MAX_ARG_SIZE);
    char out[MAX_OUTPUT_SIZE];
    bzero(out, MAX_OUTPUT_SIZE);
    
    if(strncmp(curr_co->curr_args[0], "/", 1 )!=0) {
        sprintf(new_path, "%s/%s", curr_co->pwd, curr_co->curr_args[0]);
    } else {
        sprintf(new_path, "%s/%s", curr_co->root, curr_co->curr_args[0]);
    }
    printf("%s\n", new_path);
    sprintf(cd, "cd %s && pwd", new_path);
    execute_system_cmd(cd,out, MAX_OUTPUT_SIZE);
    printf("Return from syscall : %s\n", out);
    
    if(strlen(out) == 0) {
        sprintf(curr_co->curr_out, "Invalid path");
        return 0;
    }

    if(strncmp(out, curr_co->root, strlen(curr_co->root)) != 0) {
        return ERROR_ACCESS_DENIED;
    }

    std::string relative_path(out);
    findAndReplaceAll(relative_path, curr_co->root, "");
    std::cout << "Relative path : " << relative_path << std::endl;
    sprintf(curr_co->relative_pwd, relative_path.c_str(), MAX_PATH_SIZE);
    sprintf(curr_co->pwd, out, MAX_PATH_SIZE + MAX_ROOT_PATH);
    if(relative_path.size() <= 0) {
        sprintf(curr_co->curr_out, "/");
    } else {
        sprintf(curr_co->curr_out, curr_co->relative_pwd, MAX_PATH_SIZE);
    }

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

int cmd_put(connection_t* curr_co){
    int error = 0;
    // if the client is currently using ftp
    check_ftp(&(curr_co->ftp_data));
    // create the new file path 
    strncpy(curr_co->ftp_data.filepath,curr_co->pwd,MAX_ROOT_PATH + MAX_PATH_SIZE);
    strcat(curr_co->ftp_data.filepath,"/");
    strncat(curr_co->ftp_data.filepath,curr_co->curr_args[0],
           (MAX_ROOT_PATH + MAX_PATH_SIZE) - strlen(curr_co->ftp_data.filepath));
    // set the file size received in command
    curr_co->ftp_data.file_size = atoi(curr_co->curr_args[1]);
    // setup connection for ftp
    error = setup_ftp_connection_server(curr_co);
    if(error){
        printf("Error : setup ftp connection failed \n");
        return error;
    }
    // setup other ftp related fields
    curr_co->ftp_data.using_ftp = true;
    curr_co->ftp_data.ftp_type = FTP_RECV;
    curr_co->ftp_data.ftp_user = FTP_SERVER;
    curr_co->ftp_data.main_socket = curr_co->connection_socket;
    //spawn thread
    pthread_create(&(curr_co->ftp_data.ftp_id),NULL,ftp_subthread,(void *) &(curr_co->ftp_data));
    //answer to client
    sprintf(curr_co->curr_out,"put port:    %d",curr_co->ftp_data.ftp_port);
    return 0;
}

int cmd_get(connection_t* curr_co){
    int error = 0;
    size_t file_size;
    check_ftp(&(curr_co->ftp_data));
    strncpy(curr_co->ftp_data.filepath,curr_co->pwd,MAX_ROOT_PATH + MAX_PATH_SIZE);
    strcat(curr_co->ftp_data.filepath,"/");
    strncat(curr_co->ftp_data.filepath,curr_co->curr_args[0],
           (MAX_ROOT_PATH + MAX_PATH_SIZE) - strlen(curr_co->ftp_data.filepath));
    struct stat path_stat;
    stat(curr_co->ftp_data.filepath, &path_stat);
    // check if file exists or not or is directory
    if(!S_ISREG(path_stat.st_mode)){
        return ERROR_FILE_NOT_FOUND;
    }
    // open the file to seek the file size
    FILE* file_to_send = fopen(curr_co->ftp_data.filepath,"rb");
    if(file_to_send == NULL){
        // if pointer is null then probably file was not found
        return ERROR_FILE_NOT_FOUND;
    }
    fseek(file_to_send, 0L, SEEK_END);
    file_size = ftell(file_to_send);
    fclose(file_to_send);
    // setup connection for ftp
    error = setup_ftp_connection_server(curr_co);
    if(error){
        printf("Error : setup ftp connection failed \n");
        return error;
    }
    // setup other ftp related fields
    curr_co->ftp_data.using_ftp = true;
    curr_co->ftp_data.ftp_type = FTP_SEND;
    curr_co->ftp_data.ftp_user = FTP_SERVER;
    curr_co->ftp_data.main_socket = curr_co->connection_socket;
    curr_co->ftp_data.file_size = file_size;
    // spawn ftp thread
    pthread_create(&(curr_co->ftp_data.ftp_id),NULL,ftp_subthread,(void *) &(curr_co->ftp_data));
    // answer to client with port and size of file
    sprintf(curr_co->curr_out,"get port:  %d size:  %zu",curr_co->ftp_data.ftp_port,file_size);
    return 0;
}

int cmd_grep(connection_t* curr_co) {
    printf("--- grep ---\n");
    char cmd[MAX_ARG_SIZE + MAX_ROOT_PATH + MAX_PATH_SIZE + MAX_MARGIN];
    bzero (cmd, MAX_ARG_SIZE + MAX_ROOT_PATH + MAX_PATH_SIZE + MAX_MARGIN);
    sprintf(cmd, "grep %s %s -rl",curr_co->curr_args[0], curr_co->pwd);
    std::cout << cmd << std::endl;
    char out[4*MAX_OUTPUT_SIZE];
    bzero(out, 4*MAX_OUTPUT_SIZE);
    execute_system_cmd(cmd,out,4*MAX_OUTPUT_SIZE);
    if(strlen(out) > 0) {
        std::string tmp(out);
        findAndReplaceAll(tmp,curr_co->root,"");
        strncpy(curr_co->curr_out, tmp.c_str(), MAX_OUTPUT_SIZE);
    } else {
        strcpy(curr_co->curr_out, "No match");
    }
    return 0;
}

int tokenize_cmd(char *in, char (*out)[MAX_ARG_SIZE] ){
    int i = 0;
    int token_num = 0;
    char input[MAX_INPUT_SIZE];
    strncpy(input,in,MAX_INPUT_SIZE);
    char* token = strtok(input, " ");
    while(token != NULL && i < MAX_TOKENS){
        if(strlen(token) > MAX_ARG_SIZE){
            return ERROR_ARGUMENT_SIZE;
        }
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
    char* token = strtok(path, "/");
    while(token != NULL){
        token_num ++;
        strncpy(out[i],token,MAX_ARG_SIZE);
        token = strtok(NULL,"/");      
        i += 1;
    }
    return token_num;
}