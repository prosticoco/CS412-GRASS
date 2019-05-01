#ifndef ERROR_H
#define ERROR_H

/**
 * @brief list of all different error values we  use
 * to handle errors in the project.
 * 
 */
 enum{
    ERROR_NETWORK= -128,
    ERROR_SOCKET,
    ERROR_NULL,
    ERROR_CMD,
    ERROR_TOKEN,
    ERROR_IO,
    ERROR_BIND,
    ERROR_SEND,
    ERROR_RECV,
    ERROR_FILESIZE,
    ERROR_OUTPUT,
    ERROR_FOLDER_NAME_SIZE,
    ERROR_INVALID_PATH,
    ERROR_ACCESS_DENIED,
    ERROR_MAX_PATH_SIZE,
    ERROR_FILE_NOT_FOUND,
    ERROR_DIRECTORY,
    ERROR_ARGUMENT_SIZE
};



#endif