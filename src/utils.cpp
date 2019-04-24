
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
    char * error= NULL;
    char tmp[MAX_OUTPUT_SIZE];
    bzero(tmp,MAX_OUTPUT_SIZE);
    size_t total = 0;
    total = fread(tmp,1,MAX_OUTPUT_SIZE-1,out);
    if(total == 0){
        printf("Test : total is zero \n");
    }
    if(total < 0){
        printf("Error reading pipe \n");
        pclose(out);
        return ERROR_IO;
    }
    output[total] = '\0';
    sprintf(output,tmp);
    if(output[strlen(output)-1] == '\n'){
        output[strlen(output)-1] = '\0';
    }
    printf("output: [%s]\n",output);
    //int error = pclose(out)/256;
    pclose(out);
    return 0;
}