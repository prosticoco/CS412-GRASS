#ifndef GRASS_H
#define GRASS_H

#define DEBUG true

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct {
    const char* uname;
    const char* pass;
    bool isLoggedIn;
}user_t;



typedef struct {
    const char* cname;
    const char* cmd;
    const char* params;
    bool authent;
}command_t;

typedef struct {
    size_t num_commands;
    command_t* commands;
}command_list_t;

typedef struct {

}user_list_t;

void hijack_flow();

#endif
