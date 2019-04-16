
#include <stdio.h>
#include <pthread.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
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
    char * err = fgets(output,size,out);
    if(!err){
        printf("Error with fgets in execute_system_cmd \n");
        fclose(out);
        return ERROR_IO;
    }
    fclose(out);
    return 0;
}