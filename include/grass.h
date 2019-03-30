#ifndef GRASS_H
#define GRASS_H

#define DEBUG true

#include <string>
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
#include <vector>

typedef struct {
    const std::string uname;
    const std::string pass;
    bool isLoggedIn;
}user_t;


typedef int (*grass_fct) (char **);

typedef struct {
    const char* name;
    size_t num_params;
    bool authent;
    grass_fct fct;
}command_t;

typedef struct {
    size_t num_commands;
    command_t* commands;
}command_list_t;

typedef struct {

}user_list_t;


typedef struct{
    int main_socket;
    int main_portno;
    bool login_start;
    std::string base_dir;
    std::vector<user_t> users;
}data_t;

void hijack_flow();

#endif
