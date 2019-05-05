#ifndef GRASS_H
#define GRASS_H

#define DEBUG true

/**
 * @file grass.h
 * @author Adrien Prost and Rodrigo Granja
 * @brief 
 * This file contains all constants used in the client and server code
 * as well as most defined structures used to control the data 
 * which we call metadata structures
 * @version 0.1
 * @date 2019-05-05
 * 
 */

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

/**
 * @brief ALL DEFINED CONSTANTS CAN BE FOUND HERE
 * These constants define :
 * - Maximum sizes for various buffers
 * - File size constraints for put/get to avoid usage of too many ressources on the system
 * - Constant name for the root directory the client has access to
 * - Chunk size used for receiving files
 * - Integer values to differentiate Send/Recv in the ftp code
 * 
 */
#define MAX_ARG_SIZE 128
#define MAX_USERNAME_SIZE 32
#define MAX_PASSWORD_SIZE 32
#define MAX_THREAD_NUMS 16
#define IP_ADDRESS_MAX_LENGTH 16
#define MAX_TOKENS 6
#define MAX_TOKENS_PATH 15
#define MAX_INPUT_SIZE 256
#define MAX_OUTPUT_SIZE 512
#define MAX_PATTERN_SIZE 64
#define MAX_LINE_SIZE 128
#define MAX_PATH_SIZE 128
#define MAX_ROOT_PATH 256
#define MAX_FILE_SIZE 4294967296
#define MAX_FILENAME_SIZE 32
#define MAX_FOLDER_NAME_SIZE 32
#define MAX_MARGIN 32
#define MAX_LENGTH 16
#define ROOT_DIR_NAME "/root"
#define ROOT "root"
#define FTP_CLIENT 0
#define FTP_SERVER 1
#define CHUNK_SIZE 256
#define MAX_CO_FTP 1
#define DICT_SIZE 8
#define SEND 0
#define RECV 1


/**
 * @brief Various typedefs for structures which are explained more in detail below
 * 
 */
struct connection_t;
typedef struct connection_t connection_t;
struct ftp_data_t;
typedef struct ftp_data_t ftp_data_t;
struct client_t;
typedef struct client_t client_t;
struct ftp_thread_t;
typedef struct ftp_thread_t ftp_thread_t;


/**
 * @brief Structure which represents a user for which the username and password are stored in the 
 * grass.conf file. this structure also keeps track if the user is currently logged in or not
 * 
 */
typedef struct {
    char* uname;
    char* pass;
    bool isLoggedIn;
}user_t;

/**
 * @brief function pointer type which defines command related functions
 * 
 */
typedef int (*grass_fct) (connection_t *);

/**
 * @brief structure which represents a command by its name, the number of parameters it requires
 * a pointer to its related function and a boolean which indicates if the command requires authentication
 * to be invoked
 * 
 */
typedef struct {
    const char* name;
    size_t num_params;
    bool authent;
    grass_fct fct;
}command_t;

/**
 * @brief this structure represents the list of available commands
 * it is defined explicitily as a global variable in cmd.cpp
 * 
 */
typedef struct {
    size_t num_commands;
    command_t* commands;
}command_list_t;

/**
 * @brief this structure represents the metadata used by the server.
 * It comprises of :
 * -main thread id (the thread which frees up the memory when the program is stopped)
 * -main portnumber which is defined in the config file
 * -string representing the directory name stored in the config file
 * -a list of users which represents the users for which their username and password is stored in the config file
 * -list of active connections to the server. represented by the connection_t structure explained below.
 * -mutex lock to protect the list of connections (since data races can occur when a connection is inserted/deleted from the list)
 * -absolute path to the server's directory
 * 
 */
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

/**
 * @brief Metadata used for ftp connections used in client and server code 
 * it comprises of :
 * -two thread id's, one which receives files and the other which sends files.
 * -mutex lock to prevent used when changing critical values of this structure (on which race conditions may occur)
 * -various socket file descriptor : one for the main socket (the one used for commands), one for transfering the files, as well as one for TCP handshake
 * -current port used for transfering files.
 * -filepaths to current file that is being downloaded as well as uploaded
 * -ip address of the server
 * -boolean values indicating if the port/socket is open for transfering files as well as if the user is currently sending/receiving a file
 * -file pointers to the files being downloaded/uploaded
 * -file sizes
 * 
 */
struct ftp_data_t{
    pthread_t send_id;
    pthread_t recv_id;
    pthread_mutex_t clean_lock;
    int main_socket;
    int file_transfer_socket;
    int ftp_socket;
    int ftp_port;
    int ftp_user;
    char filepath_recv[MAX_ROOT_PATH + MAX_PATH_SIZE];
    char filepath_send[MAX_ROOT_PATH + MAX_PATH_SIZE];
    char ip[IP_ADDRESS_MAX_LENGTH];
    bool port_open;
    bool sending;
    bool receiving;
    FILE * ftp_file_recv;
    FILE * ftp_file_send;
    size_t file_size_recv;
    size_t file_size_send;
};

/**
 * @brief structure representing the metada of an active connection
 * it comprises of :
 * -thread id which is communicating to client
 * -pointer to the server metadata which sometimes need to be accessed
 * -main communication socket, used for commands interaction with the client
 * -username of the client, will be assigned randomly if not logged int
 * -various boolean values to keep track if current client is authenticated, if the last command used is login, if the client exits
 * -two pointers to buffers representing the input from the client, and the response from the server
 * -various paths for client directory management
 * -pointer to tokenized arguments from the client
 * -ftp metadata
 * 
 */
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


/**
 * @brief Main metadata structure used for the client code
 * comprises of :
 * -thread ids of communication,file sending,file receiving
 * -mutex lock to handle dataraces
 * -current working directory path for file management
 * -portnumber for connection to server
 * -ftp metadata
 * 
 */
struct client_t{
    pthread_t reader;
    pthread_t writer;
    pthread_t main;
    pthread_mutex_t lock;
    char cwd[MAX_ROOT_PATH];
    int portno;
    ftp_data_t ftp_data;
};


/**
 * @brief global variable to server metadata
 * 
 */
extern data_t* prog_data;

/**
 * @brief function to hijack to successfully exploit the code
 * 
 */
void hijack_flow();


// SERVER RELATED FUNCTIONS

/**
 * @brief function which maintains command communication between server and one client
 * 
 * @param curr_co the client metadata (will be cast to connection_t* pointer)
 * @return void* thread exits if function ends
 */
void *handle_client(void* curr_co);

/**
 * @brief Initializing function for any new client connection
 * Initialized the new clients metadata before communication
 * 
 * @param socket the socket on which the client will communicate
 * @param co the connection metadata to be initialized
 * @param data the server metadata
 * @return int 0 on success negative error code otherwise
 */
int init_connection(int socket,connection_t* co,data_t* data);


/**
 * @brief Listens to the main socket for new TCP connections and then calls connection handler
 * 
 * @param data pointer the program's data
 */
void accept_connections(data_t* data);


/**
 * @brief parses the grass.conf file and updates the fields in the server's metadata
 * 
 * @param data 
 */
void parse_grass(data_t * data);


// CLIENT CODE FUNCTION PROTOTYPES

/**
 * @brief thread function which reads the servers responses and acts accordingly from the client side
 * 
 * @param ptr pointer to the client metadata (is cast to client_t *)
 * @return void* thread exits if function ends
 */
void *client_reader(void* ptr);

/**
 * @brief thread function which gets input commands from user, and then sends the commands to the server,
 * also acts accordingly w.r to the command
 * 
 * @param ptr pointer to the client metadata (is cast to client_t *)
 * @return void*  thread exits if function ends
 */
void *client_writer(void* ptr);

/**
 * @brief exit function for the client, stops all threads and closes any file/socket which is still open
 * 
 * @param signum signal number which triggered the function
 */
void stop_and_clean(int signum);

/**
 * @brief checks the input given by the user, more precisely checks if the user is issuing a put/get command
 * and acts accordingly
 * 
 * @param input the input from the user
 * @param client the client metadata
 * @return int the number of tokens the user gave as input
 */
int check_input(char* input,client_t* client);

/**
 * @brief checks the response from the server, more precisely checks if the server is answering to any get/put request
 * and acts accordingly, setting the ftp portnumber and spawning the ftp related threads
 * 
 * @param response the response given by the server
 * @param client client metadata
 * @return int the number of tokens in the response given by the server
 */
int check_response(char* response,client_t* client);

/**
 * @brief Function which initializes the client code, more precisely updates the current working directory and
 * connects to the server specified by the arguments of the program
 * 
 * @param client client metadata to be intialized
 * @param argv arguments given the client program
 * @return int 0 on success, negative error code if failure
 */
int init(client_t* client,char** argv);



#endif
