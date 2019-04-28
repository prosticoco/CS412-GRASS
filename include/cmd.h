#ifndef CMD_H
#define CMD_H

#include "grass.h"


/**
 * @brief Main client-input handler, for every line sent by the client which is read from the
 * socket, the corresponding thread calls this functions which handles the command in the following way :
 * 1. Tokenize the input according to whitespaces
 * 2. Find the corresponding command, if command is not in our list of commands, the function 
 * returns an error
 * 3. Verify that the number arguments tokens coincide with the number of arguments a command has
 * 4. Check if the requested command needs authentification to be invoked, and if so
 * it will check that the user is indeed authenticated
 * 5. if nothing else failed, this function will call the function corresponding to the command requested
 * 
 * @param curr_co pointer to the metadata of the connection
 * @return int returns 0 on success, negative value if any error occured
 */
int process_cmd(connection_t* curr_co);

/**
 * @brief Tokenizes an input according to whitespace into an array of character-arrays
 * 
 * @param cmd the input command to tokenize
 * @param out the prepared array which will contain the tokens
 * @return int returns the number of tokens
 */
int tokenize_cmd(char* cmd,char (*out)[MAX_ARG_SIZE]);

/**
 * @brief Tokenizes an input according to '/' into an array of character-arrays
 * 
 * @param path the input command to tokenize
 * @param out the prepared array which will contain the tokens
 * @return int returns the number of tokens
 */
int tokenize_path(char* path, char (*out)[MAX_FOLDER_NAME_SIZE]);

/**
 * @brief function which performs the login command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_login(connection_t* curr_co);

/**
 * @brief function which performs the pass command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_pass(connection_t* curr_co);

/**
 * @brief function which performs the ping command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_ping(connection_t* curr_co);

/**
 * @brief function which performs the date command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_date(connection_t* curr_co);

/**
 * @brief function which performs the whoami command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_whoami(connection_t* curr_co);

/**
 * @brief function which performs the w command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_w(connection_t* curr_co);

/**
 * @brief function which performs the logout command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_logout(connection_t* curr_co);

/**
 * @brief function which performs the exit command 
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_exit(connection_t* curr_co);

/**
 * @brief function which performs the ls command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_ls(connection_t* curr_co);

/**
 * @brief function which performs the cd command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_cd(connection_t* curr_co);

/**
 * @brief function which performs the mkdir command
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_mkdir(connection_t* curr_co);

/**
 * @brief function which performs the rm command ( restricted to the current working directory)
 * 
 * @param curr_co pointer to the metadata of the connection 
 * @return int returns 0 upon success, negative value if an error occured
 */
int cmd_rm(connection_t* curr_co);

int cmd_get(connection_t* curr_co);
int cmd_put(connection_t* curr_co);

#endif