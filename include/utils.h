#ifndef UTILS_H
#define UTILS_H


#include "grass.h"

/**
 * @brief safely adds (using a mutex) a connection pointer to the vector keeping track of
 * all connections
 * 
 * @param data pointer to the server data
 * @param connection pointer to the connection we wish to remove
 */
void add_connection(data_t* data, connection_t * connection);


/**
 * @brief safely removes (using a mutex) a connection pointer from the vector keeping track of all
 * connections
 * 
 * @param data pointer to the server data
 * @param connection pointer to the connection we wish to remove
 */
void remove_connection(data_t * data, connection_t * connection);

/**
 * @brief executes a unix command specified in cmd and stores the output of the command
 * in output 
 * 
 * @param cmd the command to execute
 * @param output pointer to the output of the command
 * @param size the maximum size the command can have
 * @return int returns negative value upon error and 0 if successfull
 */
int execute_system_cmd(const char *cmd,char* output,size_t size);

#endif