
#include "utils.h"
#include "grass.h"
#include <pthread.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>


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