
#include "cmd.h"
#include "error.h"
#include "utils.h"
#include "ftp.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <algorithm>
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
    curr_co->curr_in[strlen(curr_co->curr_in)-1] = '\0';
    char splitted_cmd[MAX_TOKENS][MAX_ARG_SIZE];
    for (int i = 0; i < MAX_TOKENS; i++) {
        bzero(splitted_cmd[i], MAX_ARG_SIZE);
    }
    int num_tokens = tokenize_cmd(curr_co->curr_in,splitted_cmd);
    if(num_tokens == 0){
        return 0;
    }
    if(num_tokens < 0){
        return num_tokens;
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
                    fflush(stdout);

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
    printf("Login attempt\n");
    if(strlen(curr_co->curr_args[0])>= MAX_USERNAME_SIZE) {
        return ERROR_USERNAME_SIZE;
    }
    curr_co->ready_for_check = true;
    strncpy(curr_co->tmp_username,curr_co->curr_args[0],MAX_USERNAME_SIZE);
    strcpy(curr_co->curr_out,"Please provide your password");
    return 0;
}

int cmd_pass(connection_t * curr_co){
    if(strlen(curr_co->curr_args[0]) >= MAX_PASSWORD_SIZE) {
        return ERROR_PASSWORD_SIZE;
    }

    if(curr_co->auth) {
        strcpy(curr_co->curr_out,"User already authentified");
        return 0;
    }
    if(!(curr_co->ready_for_check)){
        strcpy(curr_co->curr_out,"Please provide a username first");
        return 0;
    }    
    bool found = false;
    char username[MAX_USERNAME_SIZE];
    bzero(username,MAX_USERNAME_SIZE);
    strncpy(username,curr_co->tmp_username,MAX_USERNAME_SIZE);
    for(auto& user : curr_co->server_data->users) {
        if( strncmp(user->uname, username, strlen(user->uname)) == 0) {
            if(strncmp(curr_co->curr_args[0], user->pass,  strlen(user->pass))== 0) {
                curr_co->auth = true;
                strncpy(curr_co->username, username, MAX_USERNAME_SIZE);
                strncpy(curr_co->curr_out,"Authentication successful", MAX_ARG_SIZE);
                found = true;
                break;
            }
        }
        
    }
    if(!found) {
        printf("Login failed\n");
        sprintf(curr_co->curr_out,username);
        strcat(curr_co->curr_out," : Invalid Credentials");  
    }
    printf("[%s] : login\n", curr_co->username);

    curr_co->ready_for_check = false;
    return 0;
}


bool iequals(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}

int cmd_w(connection_t* curr_co) {
    printf("[%s] : w\n", curr_co->username);
    // there is always at least one user authentified
    std::vector<std::string> usernames(0);
    for (auto co : curr_co->server_data->connections) {
        if (co->auth) {
            usernames.push_back(co->username);
        }
    }
    
    sort(usernames.begin(), usernames.end(), iequals);
    for(auto user : usernames) {
        strncat(curr_co->curr_out, user.c_str(), MAX_USERNAME_SIZE);
        strcat(curr_co->curr_out, " ");
    }
    return 0;
}


int cmd_ping(connection_t* curr_co) {
    if(curr_co->auth) {
        printf("[%s] : ping ", curr_co->username);
    } else {
        printf("User not logged in : ping ");     
    }
    if(strlen(curr_co->curr_args[0]) >= MAX_ARG_SIZE) {
        printf("- FAIL");
        return ERROR_ARGUMENT_SIZE;
    }

    printf("%s\n", curr_co->curr_args[0]);

    char cmd[MAX_INPUT_SIZE];
    bzero(cmd,MAX_INPUT_SIZE);
    sprintf(cmd, "ping %s -c 1", curr_co->curr_args[0]);
    char out[MAX_INPUT_SIZE];
    bzero(out,MAX_INPUT_SIZE);
    int err = execute_system_cmd(cmd, out);
    strncpy(curr_co->curr_out, out, MAX_INPUT_SIZE);

    return err;
}

int cmd_date(connection_t* curr_co) {
    printf("[%s] : date\n", curr_co->username);
    char out[MAX_INPUT_SIZE];
    bzero(out, MAX_INPUT_SIZE);
    int err = execute_system_cmd("date", out);
    strncpy(curr_co->curr_out, out, MAX_INPUT_SIZE);
    return err;
}

int cmd_whoami(connection_t* curr_co) {
    printf("[%s] : whoami\n", curr_co->username);
    strncpy(curr_co->curr_out, curr_co->username, MAX_USERNAME_SIZE);
    return 0;
}

int cmd_logout(connection_t* curr_co) {
    printf("[%s] : logout\n", curr_co->username);
    curr_co->auth = false;
    strncpy(curr_co->pwd, curr_co->root, MAX_PATH_SIZE);
    strncpy(curr_co->curr_out, "User is logged out", MAX_ARG_SIZE);
    return 0;
}

int cmd_exit(connection_t* curr_co) {
    if(curr_co->auth) {
        printf("[%s] : exit\n", curr_co->username);
    } else {
        printf("User not logged in : exit ");
    
    }
    curr_co->exit = true;
    strncpy(curr_co->curr_out, "", MAX_ARG_SIZE);
    return 0;
}

int cmd_ls(connection_t* curr_co) {
    printf("[%s] : ls\n", curr_co->username);
    char cmd[MAX_INPUT_SIZE] = "ls -l ";
    strncat(cmd, curr_co->pwd, MAX_PATH_SIZE);
    char out[MAX_INPUT_SIZE];
    bzero(out, MAX_INPUT_SIZE);
    
    int err = execute_system_cmd(cmd, out);
    strncpy(curr_co->curr_out, out, MAX_INPUT_SIZE);
    return err;
}

int cmd_mkdir(connection_t* curr_co) {
    // from the description : "creates a new directory with the specified name in the current working directory"
    // so if the argument is a path we throw an error because we expect a folder name
    printf("[%s] : mkdir ", curr_co->username);

    std::string tmp ( curr_co->curr_args[0]);
    
    if (tmp.find("/") != std::string::npos) {
        // its a path since it contains /
        printf("- FAIL\n");
        return ERROR_PATH_NOT_SUPPORTED;
    }

    //check for potential command injection
    if(!checkInvalidChars(curr_co->curr_args[0])) {
        return ERROR_INVALID_CHARS;
    }

    if(strlen(curr_co->curr_args[0]) >= MAX_FOLDER_NAME_SIZE) {
        printf("- FAIL\n");
        return ERROR_FOLDER_NAME_SIZE;
    }
    printf("%s\n", curr_co->curr_args[0]);

    char cmd[MAX_INPUT_SIZE] = "mkdir ";
    char dir_path[MAX_PATH_SIZE];
    bzero(dir_path, MAX_PATH_SIZE);
    strncat(dir_path, curr_co->pwd, MAX_PATH_SIZE);    
    strcat(dir_path, "/" );
    strncat(dir_path, curr_co->curr_args[0], MAX_PATH_SIZE);
    strcat(cmd, dir_path);

    char out[MAX_INPUT_SIZE];
    bzero(out,MAX_INPUT_SIZE);
    int err = mkdir(dir_path, ACCESSPERMS);
    if (err) {
        //fail
        strcpy(curr_co->curr_out, "Mkdir failed to create directory");
    } else {
        strcpy(curr_co->curr_out, "Creation successful");
    }

    return err;
}

int cmd_cd(connection_t* curr_co) {
    printf("[%s] : cd ", curr_co->username);
    if(strlen(curr_co->curr_args[0]) > MAX_PATH_SIZE) {
        printf("- FAIL\n");
        return ERROR_MAX_PATH_SIZE;
    }

    printf("%s\n", curr_co->curr_args[0]);

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
    sprintf(cd, "cd %s && pwd", new_path);
    execute_system_cmd(cd,out);
    
    if(strlen(out) == 0) {
        printf("- FAIL\n");
        sprintf(curr_co->curr_out, "Invalid path");
        return 0;
    }

    if(strncmp(out, curr_co->root, strlen(curr_co->root)) != 0) {
        printf("- FAIL\n");
        return ERROR_ACCESS_DENIED;
    }

    std::string relative_path(out);
    findAndReplaceAll(relative_path, curr_co->root, "");
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
    printf("[%s] : rm ", curr_co->username);
    
    // from the description : "deletes the file or directory with the specified name in the current working directory"
    // so if the argument is a path we throw an error because we expect a folder/file name
      std::string tmp ( curr_co->curr_args[0]);
    if (tmp.find("/") != std::string::npos) {
        // its a path since it contains '/
        printf("- FAIL\n");
        return ERROR_PATH_NOT_SUPPORTED;
    } 
    //check for potential command injection
    if(!checkInvalidChars(curr_co->curr_args[0])) {
        printf("- FAIL\n");
        return ERROR_INVALID_CHARS;
    }
    
    if(strlen(curr_co->curr_args[0]) >= MAX_FOLDER_NAME_SIZE) {
        printf("- FAIL\n");
        return ERROR_FOLDER_NAME_SIZE;
    }

    printf("%s\n", curr_co->curr_args[0]);
    char cmd[MAX_INPUT_SIZE];
    bzero(cmd, MAX_INPUT_SIZE);
    strcpy(cmd, "rm -r ");
    strncat(cmd, curr_co->pwd, MAX_PATH_SIZE);
    strcat(cmd, "/");
    strncat(cmd, curr_co->curr_args[0], MAX_FOLDER_NAME_SIZE);
    char out[MAX_OUTPUT_SIZE];
    bzero(out, MAX_OUTPUT_SIZE);
    execute_system_cmd(cmd, out);
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
    check_ftp(&(curr_co->ftp_data),RECV,true,false);
    // create the new file path 
    bzero(curr_co->ftp_data.filepath_recv,MAX_ROOT_PATH + MAX_PATH_SIZE);
    strncpy(curr_co->ftp_data.filepath_recv,curr_co->pwd,MAX_ROOT_PATH + MAX_PATH_SIZE);
    strcat(curr_co->ftp_data.filepath_recv,"/");
    strncat(curr_co->ftp_data.filepath_recv,curr_co->curr_args[0],
           (MAX_ROOT_PATH + MAX_PATH_SIZE) - strlen(curr_co->ftp_data.filepath_recv));
    // set the file size received in command
    curr_co->ftp_data.file_size_recv = atoi(curr_co->curr_args[1]);
    // setup connection for ftp
    error = setup_ftp(curr_co);
    if(error){
        printf("Error : setup ftp connection failed \n");
        return error;
    }
    // setup other ftp related fields
    curr_co->ftp_data.receiving = true;
    curr_co->ftp_data.ftp_user = FTP_SERVER;
    curr_co->ftp_data.main_socket = curr_co->connection_socket;
    //spawn thread
    pthread_create(&(curr_co->ftp_data.recv_id),NULL,ftp_thread_recv,(void *) &(curr_co->ftp_data));
    //answer to client
    sprintf(curr_co->curr_out,"put port:    %d",curr_co->ftp_data.ftp_port);
    return 0;
}

int cmd_get(connection_t* curr_co){
    int error = 0;
    size_t file_size;
    check_ftp(&(curr_co->ftp_data),SEND,true,false);
    bzero(curr_co->ftp_data.filepath_send,MAX_ROOT_PATH + MAX_PATH_SIZE);
    strncpy(curr_co->ftp_data.filepath_send,curr_co->pwd,MAX_ROOT_PATH + MAX_PATH_SIZE);
    strcat(curr_co->ftp_data.filepath_send,"/");
    strncat(curr_co->ftp_data.filepath_send,curr_co->curr_args[0],
           (MAX_ROOT_PATH + MAX_PATH_SIZE) - strlen(curr_co->ftp_data.filepath_send));
    struct stat path_stat;
    stat(curr_co->ftp_data.filepath_send, &path_stat);
    // check if file exists or not or is directory
    if(!S_ISREG(path_stat.st_mode)){
        printf("not a file : path [%s]\n",curr_co->ftp_data.filepath_send);
        return ERROR_FILE_NOT_FOUND;
    }
    // open the file to seek the file size
    FILE* file_to_send = fopen(curr_co->ftp_data.filepath_send,"rb");
    if(file_to_send == NULL){
        printf("fopen failed : path [%s]\n",curr_co->ftp_data.filepath_send);
        // if pointer is null then probably file was not found
        return ERROR_FILE_NOT_FOUND;
    }
    fseek(file_to_send, 0L, SEEK_END);
    file_size = ftell(file_to_send);
    fclose(file_to_send);
    // setup connection for ftp
    error = setup_ftp(curr_co);
    if(error){
        printf("Error : setup ftp connection failed \n");
        return error;
    }
    // setup other ftp related fields
    curr_co->ftp_data.sending = true;
    curr_co->ftp_data.ftp_user = FTP_SERVER;
    curr_co->ftp_data.main_socket = curr_co->connection_socket;
    curr_co->ftp_data.file_size_send = file_size;
    // spawn ftp thread
    pthread_create(&(curr_co->ftp_data.send_id),NULL,ftp_thread_send,(void *) &(curr_co->ftp_data));
    // answer to client with port and size of file
    sprintf(curr_co->curr_out,"get port:  %d size:  %zu",curr_co->ftp_data.ftp_port,file_size);
    return 0;
}

int cmd_grep(connection_t* curr_co) {
    printf("[%s] : grep ", curr_co->username);

    if(strlen(curr_co->curr_args[0]) >= MAX_ARG_SIZE ) {
        printf("- FAIL\n");
        return ERROR_ARGUMENT_SIZE;
    }
    //check for potential command injection
    if(!checkInvalidChars(curr_co->curr_args[0])) {
        return ERROR_INVALID_CHARS;
    }

    printf("%s\n", curr_co->curr_args[0]);
    char cmd[MAX_ARG_SIZE + MAX_ROOT_PATH + MAX_PATH_SIZE + MAX_MARGIN];
    bzero (cmd, MAX_ARG_SIZE + MAX_ROOT_PATH + MAX_PATH_SIZE + MAX_MARGIN);
    sprintf(cmd, "grep %s %s -rl",curr_co->curr_args[0], curr_co->pwd);
    char out[4*MAX_OUTPUT_SIZE];
    bzero(out, 4*MAX_OUTPUT_SIZE);
    int err = execute_system_cmd(cmd,out);
    if(strlen(out) > 0) {
        std::string tmp(out);
        findAndReplaceAll(tmp,curr_co->root,"");
        strncpy(curr_co->curr_out, tmp.c_str(), MAX_OUTPUT_SIZE);
    } else {
        strcpy(curr_co->curr_out, "No match");
    }
    return err;
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
        if(token[strlen(token)-1] == '\n'){
            token[strlen(token)-1] = '\0';
        }
        strncpy(out[i],token,MAX_ARG_SIZE);
        token = strtok(NULL," ");      
        i += 1;
    }
    return token_num;
}

int tokenize_path(char* path, char (*out)[MAX_FOLDER_NAME_SIZE]) {
    int i = 0;
    int token_num = 0;
    char input[MAX_INPUT_SIZE];
    bzero(input, MAX_INPUT_SIZE);
    strncpy(input,path,MAX_INPUT_SIZE);
    char* token = strtok(input, "/");
    while(token != NULL && i < MAX_TOKENS){
        printf("Curr token : [%s]\n ", token);
        if(strlen(token) > MAX_ARG_SIZE){
            return ERROR_ARGUMENT_SIZE;
        }
        token_num ++;
        strncpy(out[i],token,MAX_ARG_SIZE);
        token = strtok(NULL,"/");      
        i += 1;
    }
    printf("Tokenize succeed \n");
    return token_num;
}