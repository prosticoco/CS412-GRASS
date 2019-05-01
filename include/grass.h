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

#define MAX_ARG_SIZE 128
#define MAX_USERNAME_SIZE 32
#define MAX_PASSWORD_SIZE 32
#define MAX_THREAD_NUMS 16
#define IP_ADDRESS_MAX_LENGTH 16
#define MAX_TOKENS 5
#define MAX_TOKENS_PATH 15
#define MAX_INPUT_SIZE 256
#define MAX_OUTPUT_SIZE 512
#define MAX_LINE_SIZE 128
#define MAX_PATH_SIZE 128
#define MAX_ROOT_PATH 256
#define MAX_FILE_SIZE 4294967296
#define MAX_FILENAME_SIZE 32
#define MAX_FOLDER_NAME_SIZE 32
#define MAX_MARGIN 32
#define ROOT_DIR_NAME "/root"
#define ROOT "root"
#define FTP_SEND 0
#define FTP_RECV 1
#define FTP_CLIENT 0
#define FTP_SERVER 1

struct connection_t;
typedef struct connection_t connection_t;
struct ftp_data_t;
typedef struct ftp_data_t ftp_data_t;
struct client_t;
typedef struct client_t client_t;

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
}data_t;

struct ftp_data_t{
    pthread_t ftp_id;
    pthread_mutex_t clean_lock;
    int main_socket;
    int ftp_socket;
    int ftp_port;
    int ftp_type;
    int ftp_user;
    char filepath[MAX_ROOT_PATH + MAX_PATH_SIZE];
    char ip[IP_ADDRESS_MAX_LENGTH];
    bool using_ftp;
    FILE * ftp_file;
    size_t file_size;
};

struct connection_t{
    pthread_t tid;
    data_t * server_data;
    int connection_socket;
    char * username;
    char tmp_username[MAX_USERNAME_SIZE];
    bool auth;
    bool ready_for_check;
    bool exit;
    char * curr_in;
    char * curr_out;
    char relative_pwd[MAX_PATH_SIZE];
    char pwd[MAX_ROOT_PATH+MAX_PATH_SIZE];
    char root[MAX_ROOT_PATH];
    char (*curr_args)[MAX_ARG_SIZE]; 
    ftp_data_t ftp_data;
};


struct client_t{
    pthread_t reader;
    pthread_t writer;
    pthread_t main;
    pthread_mutex_t lock;
    char cwd[MAX_ROOT_PATH];
    int portno;
    ftp_data_t ftp_data;
};



extern data_t* prog_data;

void hijack_flow();

// prototype for client handler
void *handle_client(void* curr_co);
// prototype to create a new thread for a client
int init_connection(int socket,connection_t* co,data_t* data);


void *client_reader(void* ptr);

void *client_writer(void* ptr);

void stop_and_clean(int signum);

int check_input(char* input,client_t* client);

int check_response(char* response,client_t* client);



#endif
