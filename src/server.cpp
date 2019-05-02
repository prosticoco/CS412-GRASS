#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include "error.h"
#include "cmd.h"
#include "exit.h"
#include "grass.h"
#include "utils.h"
#include "ftp.h"


#define BUFFER_MAX_SIZE 256
#define MAX_CONNECTIONS 5


char port[7] = "31337";

using namespace std;





int init_connection(int new_sockfd,connection_t* tmp, data_t* data){
    int err = 0;
    printf("Initializing new connection... \n");
    tmp->ftp_data.ftp_socket = -1;
    tmp->ftp_data.ftp_port = -1;
    tmp->auth = false;
    tmp->connection_socket = new_sockfd;
    tmp->curr_args = NULL;
    tmp->ready_for_check = false;
    tmp->server_data = data;
    tmp->username = (char *) malloc(MAX_USERNAME_SIZE);
    tmp->exit = false;
    tmp->ftp_data.using_ftp = false;
    tmp->ftp_data.ftp_file = NULL;
    tmp->ftp_data.file_size = 0;
    int random = rand() % 1000;
    sprintf(tmp->username,"Unknown_user_%d",random);
    tmp->curr_in = NULL;
    tmp->curr_out = NULL;
    // initialise root directory
    strncpy(tmp->pwd, data->root_path, MAX_PATH_SIZE);
    strncpy(tmp->root, data->root_path, MAX_PATH_SIZE);
    printf("Connection PWD : %s\n", tmp->pwd);
    err = pthread_mutex_init(&(tmp->ftp_data.clean_lock),NULL);
    if(err){
        printf("Error Init ftp mutex \n");
        return err;
    }
    
    add_connection(data,tmp);
    int ret = pthread_create(&(tmp->tid),NULL,handle_client,(void *)tmp);
    return ret;
}


/**
 * @brief Function used by each thread to handle a client, this function will loop
 * and process each command request from the client, if the latter closes the connection
 * or enters the command "exit", then this function will halt and the thread will exit safely.
 * 
 * @param ptr, pointer to the connection_t structure representing the connection
 */
void *handle_client(void* ptr){
    connection_t * client = (connection_t*) ptr;
    int err = 0;
    char input[BUFFER_MAX_SIZE];
    char output[BUFFER_MAX_SIZE];
    printf("Connection started for user : [%s] \n",client->username);
    while(1){
        if(client->exit){
            break;
        }
        // count the number of bytes read from socket
        ssize_t b;
        bzero(input,BUFFER_MAX_SIZE);
        bzero(output,BUFFER_MAX_SIZE);
        b = read(client->connection_socket,input,sizeof(input));
        if(!b){
            printf("Client on socket : %d exiting \n",client->connection_socket);
            break;
        }
        // initialize input and output.
        client->curr_in = input;
        client->curr_out = output;
        err = process_cmd(client);
        if(err < 0){
           error_handler(err,client);
        }
        b = write(client->connection_socket,client->curr_out,strlen(client->curr_out));
        if(b<0){
            printf("Connection Error \n");
            break;
        }
    }
    thread_cleanup(client);
    return (void *) 0; 
}



/**
 * @brief Initialize the main socket which will accept new connections
 * 
 * @param data pointer to the program data
 */
int init_server(data_t * data){
    int error = 0;
    int sockfd = -1;
    int portno = data->main_portno;
    error = setup_server_co(&portno,&sockfd,false,MAX_CONNECTIONS);
    if(error){
        printf("Error setup_server_co() \n");
        return error;
    }
    data->main_socket = sockfd;
    check_dir(data);
    return 0;  
}

/**
 * @brief Listens to the main socket for new TCP connections and then calls connection handler
 * 
 * @param data pointer the program's data
 */
void accept_connections(data_t* data){   
    int err = 0;
    signal(SIGTERM, stop);
	signal(SIGINT, stop); 
        // Listen to the port and handle each connection
    while(1){
        connection_t* tmp;
        struct sockaddr_in cli_addr;
        socklen_t clilen;
        clilen = sizeof(cli_addr);
        int new_sockfd;
        new_sockfd = accept(prog_data->main_socket,(struct sockaddr *) &cli_addr,&clilen);
        if(new_sockfd < 0){
            printf("ERROR on accept\n");
        }
        tmp = (connection_t *) malloc(sizeof(connection_t));
        // Create a Thread or Child to callback connection handler
        err = init_connection(new_sockfd,tmp,data);
        if(err < 0){
            printf("Could not initialize new thread \n");
        }
        
    }
}

// Parse the grass.conf file and fill in the global variables
void parse_grass(data_t * data) {


    ifstream infile("../src/grass.conf");
    
    if(!infile.is_open()) {
        cout << "Error reading from config" << endl;
    }
    string line;

    //skip first 4 lines
    for(int i= 0; i <= 3; i++) {
        getline(infile, line);
    }
    // get base directory
    getline(infile, line);
    istringstream iss(line);
    vector<string> base_words((istream_iterator<string>(iss)),istream_iterator<string>());
    data->base_dir = base_words[1];
    base_words.clear();
    //skip next 2 lines
    getline(infile,line);
    getline(infile, line);
    // get port number
    getline(infile, line);
    istringstream portiss(line);
    vector<string> port_words((istream_iterator<string>(portiss)),istream_iterator<string>());
    data->main_portno = atoi(port_words[1].c_str());
    //skip 2 lines
    getline(infile, line);
    getline(infile, line);
    //get users information
    istringstream currss;
    while(getline(infile, line) ){
        currss = istringstream(line);
        vector<string> user_words((istream_iterator<string>(currss)),istream_iterator<string>());
        user_t * u = (user_t *) malloc(sizeof(user_t));
        u->uname = new char[MAX_USERNAME_SIZE];
        strncpy(u->uname,user_words[1].c_str(),MAX_USERNAME_SIZE);
        u->pass = new char[MAX_PASSWORD_SIZE];
        strncpy(u->pass,user_words[2].c_str(),MAX_PASSWORD_SIZE);
        u->isLoggedIn = false;
        data->users.push_back(u);
    }
}




int main() {
    printf("Allocating Ressources...\n");
    prog_data = (data_t * ) malloc(sizeof(data_t));
    prog_data->main_portno = 0;
    prog_data->main_socket = 0;
    //prog_data->base_dir = string();
    prog_data->users = vector<user_t *>();
    prog_data->connections = vector<connection_t *>();
    printf("Parsing Configuration file... \n");
    parse_grass(prog_data);
    int err = 0;
    printf("Initializing socket... \n");
    err = init_server(prog_data);
    if(err){
        printf("Error Init server : %d \n",err);
        return err;
    }
    err = pthread_mutex_init(&(prog_data->vector_protect),NULL);
    if(err){
        printf("Error Init server mutex \n");
        return err;
    }
    prog_data->main_tid = pthread_self();
    printf("Waiting for new connections... \n");
    accept_connections(prog_data);
    return 0;
}
