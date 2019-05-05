#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @author Adrien Prost and Rodrigo Granja
 * @brief contains many function prototypes which are used throughout the code
 * @version 0.1
 * @date 2019-05-05
 * 
 * @copyright Copyright (c) 2019
 * 
 */

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
int execute_system_cmd(const char *cmd,char* output);

/**
 * @brief checks if root directory is present, if it is we reset it, if it is not we create it
 * 
 * @param data the server data  
 * */
int check_dir(data_t* data);

/**
 * @brief Finds and replaces all occurrences of a given substring
 * 
 * @param data the original string
 * @param toSearch substring to find
 * @param replaceStr the substring to replace with
 * */
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr);

/**
 * @brief Handles errors
 * 
 * @param err the error code
 * @param curr_co the current client connection where the function will output the error message
 * */
void error_handler(int err,connection_t* curr_co);

/**
 * @brief Checks the given string for invalid characters
 * 
 * @param in the string to be checked
 * @param dict_size the size of the dictionnary
 * @return true if the string does not contain invalid chars, false otherwise 
 * */
bool checkInvalidChars(char * in, size_t dict_size);

/**
 * @brief prints all fields in the ftp metadata, only used for debugging
 * 
 * @param ftp ftp data pointer
 */
void print_ftp_fields(ftp_data_t * ftp);

/**
 * @brief prints all fields in the connection metadata, only used for debugging
 * 
 * @param c pointer to the connection metadata
 */
void print_connection_fields(connection_t * c);

/**
 * @brief checks that the path is valid and is pointing to a regular file
 * if so, tests if this file can be opened without any problem
 * if so,fetches the size of the file given in the path (used in get command)
 * and updates the file size field in ftp metadata
 * 
 * @param path path to a file
 * @param c metadata pointer
 * @return int returns 0 upon success, negative error code if file was not found
 */
int check_file_validity(char* path,connection_t * c);

/**
 * @brief Checks if the pattern we wish to find is valid
 * 
 * @param pattern we wish to check
 * @return int the error if there is one, 0 otherwise
 */
int check_pattern_validity(char * pattern);

#endif