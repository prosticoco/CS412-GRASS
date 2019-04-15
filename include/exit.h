#ifndef EXIT_H
#define EXIT_H

#include "grass.h"

void stop(int signum);

void thread_end();

int stop_threads(data_t * data);

int clean_memory(data_t * data);

int cleanup(data_t * data);

int thread_cleanup(connection_t* cur_co);



#endif