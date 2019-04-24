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

#define MAX_ARG_SIZE 32
#define MAX_USERNAME_SIZE 32
#define MAX_PASSWORD_SIZE 32
#define MAX_THREAD_NUMS 16
#define MAX_PATH_SIZE 128
#define ROOT_DIR_NAME "/root"

struct connection_t;
typedef struct connection_t connection_t;

typedef struct {
    char* uname;
    char* pass;
    bool isLoggedIn;
}user_t;

typedef int (*grass_fct) (connection_t *);

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
    pthread_t main_tid;
    int main_socket;
    int main_portno;
    std::string base_dir;
    std::vector<user_t*> users;
    std::vector<connection_t *> connections;
    pthread_mutex_t vector_protect;
}data_t;

struct connection_t{
    pthread_t tid;
    data_t * server_data;
    int connection_socket;
    int ftp_socket;
    int ftp_port;
    char * username;
    bool auth;
    bool ready_for_check;
    bool exit;
    char * curr_in;
    char * curr_out;
    char pwd[MAX_PATH_SIZE];
    char root[MAX_PATH_SIZE];
    char (*curr_args)[MAX_ARG_SIZE];
};


extern data_t* prog_data;

void hijack_flow();



#endif
