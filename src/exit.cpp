#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "exit.h"


void stop(int signum) {
	//reset signal handlers to default
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
    printf("Function called nicely \n");
    exit(0);
}
