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
#include "path.h"

#define MAX_ARG_SIZE 32
#define MAX_USERNAME_SIZE 32
#define MAX_PASSWORD_SIZE 32
#define MAX_THREAD_NUMS 16
#define MAX_TOKENS 5
#define MAX_TOKENS_PATH 15
#define MAX_INPUT_SIZE 256
#define MAX_OUTPUT_SIZE 512
#define MAX_LINE_SIZE 128
#define MAX_PATH_SIZE 128
#define ROOT_DIR_NAME "/root"
#define ROOT "root"

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
    char root_path[MAX_PATH_SIZE];
    Node* root;
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
    char relative_pwd[MAX_PATH_SIZE];
    char pwd[MAX_PATH_SIZE];
    char root[MAX_PATH_SIZE];
    char (*curr_args)[MAX_ARG_SIZE];
    Node* root_node;
    Node* curr_node; 
};


extern data_t* prog_data;

void hijack_flow();



#endif
