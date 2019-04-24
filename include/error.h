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
    ERROR_OUTPUT
};



#endif