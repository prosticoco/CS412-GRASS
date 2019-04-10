#ifndef EXIT_H
#define EXIT_H

#include "grass.h"

void stop(int signum);


int stop_threads(data_t * data);

int clean_memory(data_t * data);

int cleanup(data_t * data);




#endif