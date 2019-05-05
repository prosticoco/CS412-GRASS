#ifndef EXIT_H
#define EXIT_H

/**
 * @file exit.h
 * @author Adrien Prost and Rodrigo Granja (you@domain.com)
 * @brief Contains functions used to free data used by the server and kill threads
 * @version 0.1
 * @date 2019-05-05
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "grass.h"



/**
 * @brief Signal handler which takes care of exiting the program in a clean way, whenever
 * the server program is interrupted. takes care of killing any leftover thread and freeing
 * all memory allocated on the heap
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
 * This function is called whenever a client disconnects before the server is shutdown
 * 
 * @param cur_co pointer to the connection metadata
 */
void thread_cleanup(connection_t* cur_co);



#endif