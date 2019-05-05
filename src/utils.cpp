#include <stdio.h>
#include <pthread.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "utils.h"
#include "grass.h"
#include "error.h"

void add_connection(data_t * data, connection_t * connection){
    pthread_mutex_lock(&(data->vector_protect));
    data->connections.push_back(connection);
    pthread_mutex_unlock(&(data->vector_protect));
}

void remove_connection(data_t * data,connection_t * connection){
    pthread_mutex_lock(&(data->vector_protect));
    data->connections.erase(
        std::remove(data->connections.begin(),
        data->connections.end(), connection),
        data->connections.end());
    pthread_mutex_unlock(&(data->vector_protect));
}

int execute_system_cmd(const char *cmd,char* output){
    if(cmd == NULL || output == NULL){
        printf("Error NULL argument in execute_system_cmd \n");
        return ERROR_NULL;
    }
    // add this so that the output of popen prints out any error
    char * error_msg = " 2>&1";
    char command[MAX_INPUT_SIZE];
    memset(command,0,sizeof(command));
    strncat(command,cmd,strlen(cmd));
    strncat(command,error_msg,strlen(error_msg));
    FILE*  out = popen(command,"r");
    if(!out){
        printf("Error popen on command : [%s] \n",cmd);
        return ERROR_IO;
    }
    size_t total = 0;
    total = fread(output,1,MAX_OUTPUT_SIZE-1,out);
    output[total] = '\0';
    if(output[strlen(output)-1] == '\n'){
        output[strlen(output)-1] = '\0';
    }
    pclose(out);
    return 0;
}

int check_dir(data_t* data) {
    char cwd[MAX_PATH_SIZE];
    char root_path[MAX_PATH_SIZE];
    int err = 0;
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strncpy(root_path, cwd, MAX_PATH_SIZE);
        if(strcmp(data->base_dir.c_str(), ".") != 0) {
            strcat(root_path, "/");
            strcat(root_path, data->base_dir.c_str());
        }
    } else {
        return -1;
    }
    strncat(root_path, ROOT_DIR_NAME, MAX_PATH_SIZE);

    DIR* dir = opendir(root_path);
    
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        char cmd[MAX_INPUT] = "rm -r ";
        strcat(cmd, root_path);
        strcat(cmd, "/*");
        
        err = system(cmd);
    } else {
        char cmd[MAX_INPUT_SIZE]= "mkdir ";
        strcat(cmd, root_path);
        system(cmd);
        //todo handle error
    }

    strncpy(data->root_path, root_path, MAX_INPUT_SIZE);
    
    return 0;
}


void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr) {
	// Get the first occurrence
	size_t pos = data.find(toSearch);
 
	// Repeat till end is reached
	while( pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos =data.find(toSearch, pos + replaceStr.size());
	}
}

bool checkInvalidChars(char * in,size_t dict_size) {
    std::string toCheck(in);
    std::string dict("&|;$><\!`");
    for(size_t i = 0; i < dict_size; i++) {
        if (toCheck.find(dict.at(i)) != std::string::npos) {
            // contains invalid char
            return false;
        }
    }

    return true;
}

void error_handler(int err,connection_t* client){
    switch(err) {
        case ERROR_FOLDER_NAME_SIZE:
            printf("ERROR : File/folder length too long\n");
            strcpy(client->curr_out, "ERROR : File/folder length too long");
            break;
        case ERROR_INVALID_CHARS:
            printf("ERROR : Arguments contain invalid characters\n");
            strcpy(client->curr_out, "ERROR : Arguments contain invalid characters");
            break;
        case ERROR_INVALID_PATH:
            printf("ERROR : Invalid path\n");
            strcpy(client->curr_out,"ERROR : Invalid path");
            break;
        case ERROR_ACCESS_DENIED:
            printf("ERROR : Access denied\n");
            strcpy(client->curr_out,"ERROR : Access denied");
            break;
        case ERROR_FILE_NOT_FOUND:
            printf("ERROR : File not found\n");
            strcpy(client->curr_out,"ERROR : File not found");
            break;
        case ERROR_ARGUMENT_SIZE:
            printf("ERROR : Argument is too long\n");
            strcpy(client->curr_out,"ERROR : Argument is too long");
            break;
        case ERROR_DIRECTORY:
            printf("ERROR : It is a directory\n");
            strcpy(client->curr_out,"ERROR : It is a directory");
            break;
        case ERROR_USERNAME_SIZE:
            printf("ERROR : Invalid username\n");
            strcpy(client->curr_out,"ERROR : Invalid username");
            break;
        case ERROR_PASSWORD_SIZE:
            printf("ERROR : Invalid password\n");
            strcpy(client->curr_out,"ERROR : Invalid password");
            break;
        case ERROR_PATH_NOT_SUPPORTED:
            printf("ERROR : command does not support paths\n");
            strcpy(client->curr_out,"ERROR : command does not support paths");
            break;
        case ERROR_MAX_PATH_SIZE:
            printf("ERROR : command does not support paths longer than 128\n");
            strcpy(client->curr_out,"ERROR : command does not support paths longer than 128");
            break;
        case ERROR_FILESIZE:
            printf("ERROR : File size is too long\n");
            strcpy(client->curr_out,"ERROR : File size is too long");
            break;
        default :
            printf("ERROR : default error in command processing\n");
    }
}

void print_connection_fields(connection_t * c){
    printf("main thread : [%lu] \n",c->tid);
    printf("connection socket : %d \n",c->connection_socket);
    printf("username : %s \n",c->username);
    printf("tmp username : [%s]\n",c->tmp_username);
    printf("autenticated : %d\n",c->auth);
    printf("pwd : [%s]\n",c->pwd);
    print_ftp_fields(&(c->ftp_data));
}

void print_ftp_fields(ftp_data_t * ftp){
    printf("main socket : %d \n",ftp->main_socket);
    printf("file transfer socket : %d \n",ftp->file_transfer_socket);
    printf("ftp socket : %d \n",ftp->ftp_socket);
    printf("ftp port : %d \n",ftp->ftp_port);
    printf("filepath recv : [%s]\n",ftp->filepath_recv);
    printf("filepath_send : [%s]\n",ftp->filepath_send);
    printf("port is open : %d \n",ftp->port_open);
    printf("sending : %d \n",ftp->sending);
    printf("receiving : %d \n",ftp->receiving);
}