#ifndef EXIT_H
#define EXIT_H

#include "grass.h"

/**
 * @brief Signal handler which takes care of exiting the program in a clean way, whenever
 * the server program is interrupted. More detailed explanation can be found in the function
 * definition
 * 
 * @param signum signal number 
 */
void stop(int signum);

/**
 * @brief ends/exits the calling thread
 * 
 */
void thread_end();

/**
 * @brief cleans up all data only necessary to the calling thread and subsequently ends/exits the thread
 * 
 * @param cur_co pointer to the connection metadata
 */
void thread_cleanup(connection_t* cur_co);



#endif