
#include <stdio.h>
#include <pthread.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
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

int execute_system_cmd(const char *cmd,char* output,size_t size){
    if(cmd == NULL || output == NULL){
        printf("Error NULL argument in execute_system_cmd \n");
        return ERROR_NULL;
    }
    FILE*  out = popen(cmd,"r");
    if(!out){
        printf("Error popen on command : [%s] \n",cmd);
        return ERROR_IO;
    }
    size_t bytes_read = 0;
    bytes_read = fread(output,sizeof(char),size,out);
    if(bytes_read <= 0){
        printf("Error with read in execute_system_cmd \n");
        fclose(out);
        return ERROR_IO;
    }
    fclose(out);
    return 0;
}

int check_dir(connection_t* co) {
    char cwd[MAX_PATH_SIZE];
    char root_path[MAX_PATH_SIZE];
    int err = 0;
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
        strncpy(root_path, cwd, strlen(cwd));
    } else {
        return -1;
    }
    strncat(root_path, ROOT_DIR_NAME, strlen(ROOT_DIR_NAME));
    DIR* dir = opendir(root_path);
    printf("Root path : %s\n", root_path);
    
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        printf("Root dir exists and is being reset\n");
        char cmd[MAX_INPUT] = "rm -r ";
        strcat(cmd, root_path);
        strcat(cmd, "/*");
        
        err = system(cmd);
        if(err) {
            printf("Root reset failed\n");
        }

    }
       
    strncpy(co->root, root_path, MAX_PATH_SIZE);
    strncpy(co->pwd, root_path, MAX_PATH_SIZE);


}