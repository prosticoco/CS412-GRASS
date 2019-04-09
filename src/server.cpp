#include <grass.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include "error.h"
#include "cmd.h"

#define BUFFER_MAX_SIZE 256

static data_t* prog_data;
char port[7] = "31337";

using namespace std;


void test_function(connection_t* curr_co);

/**
 * @brief Default error function
 * 
 * @param msg error msg to be displayed
 */
void error(char* msg){
    perror(msg);
    exit(1);
}

/**
 * @brief Initialize the main socket which will accept new connections
 * 
 * @param data pointer to the program data
 */
int init_server(data_t * data){
    int sockfd = -1;
    int portno = data->main_portno;   
    struct sockaddr_in serv_addr;
    // open the server's socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        error("ERROR opening socket \n");
    }
    data->main_socket = sockfd;
    // initialize the server's address sructure
    bzero((char*) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // bin the server's address and port to the socket
    if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding\n");
        return -1;
    }
    // A CHANGER
    // listen to 5 maximum connexions
    if(listen(sockfd,5) != 0){
        error("ERROR listening failed\n");
        return -1;
    }  
}

/**
 * @brief Listens to the main socket for new TCP connections and then calls connection handler
 * 
 * @param data pointer the program's data
 */
void accept_connections(data_t* data){

    
        // Listen to the port and handle each connection
    while(1){
        connection_t* tmp;
        struct sockaddr_in cli_addr;
        socklen_t clilen;
        clilen = sizeof(cli_addr);
        int new_sockfd;
        new_sockfd = accept(prog_data->main_socket,(struct sockaddr *) &cli_addr,&clilen);
        if(new_sockfd < 0){
            error("ERROR on accept\n");
        }
        tmp = (connection_t *) malloc(sizeof(connection_t));
        tmp->auth = false;
        tmp->connection_socket = new_sockfd;
        tmp->curr_args = NULL;
        tmp->ready_for_check = false;
        tmp->server_data = data;
        tmp->username = new char[MAX_USERNAME_SIZE];
        tmp->curr_in = NULL;
        tmp->curr_out = NULL;
        data->connections.push_back(tmp);
        test_function(tmp);
        close(new_sockfd);
        break;

        // Create a Thread or Child to callback connection handler
    }
}


void close_connections(data_t* data){
    close(data->main_socket);
}

void init_command_list(){

}




void clean(){
    
}







// Helper function to run commands in unix.
void run_command(const char* command, int sock){
}


/*
 * Send a file to the client as its own thread
 *
 * fp: file descriptor of file to send
 * sock: socket that has already been created.
 */
void send_file(int fp, int sock) {
}

/*
 * Send a file to the server as its own thread
 *
 * fp: file descriptor of file to save to.
 * sock: socket that has already been created.
 * size: the size (in bytes) of the file to recv
 */
void recv_file(int fp, int sock, int size) {
}

// Server side REPL given a socket file descriptor
void *connection_handler(void* sockfd) {

}

/*
 * search all files in the current directory
 * and its subdirectory for the pattern
 *
 * pattern: an extended regular expressions.
 * Output: A line seperated list of matching files' addresses
 */
void search(char *pattern) {
    // TODO
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
    for(auto t : data->users){
        cout << t->uname  << " "  << t->pass << endl;
        
    }
}


/**
 * @brief Test function for starting the project, once connection is accepted
 * chat with the client YOHO
 * 
 * @param sockfd the socket for chatting
 */
void test_function(connection_t* client){
    int err = 0;
    char input[BUFFER_MAX_SIZE];
    char output[BUFFER_MAX_SIZE];
    while(1){
        ssize_t b;
        printf("Reading from socket \n");
        bzero(input,sizeof(input));
        bzero(output,sizeof(output));
        b = read(client->connection_socket,input,sizeof(input));
        if(!b){
            break;
        }
        printf("From Client : %s \n",input);
        if(strncmp("exit",input,4) == 0){
            printf("Server Exit ... \n");
            break;
        }
        client->curr_in = input;
        client->curr_out = output;
        err = process_cmd(client);
        if(err < 0){
            printf("Error processing message \n");
            return;
        }
        b = write(client->connection_socket,client->curr_out,strlen(client->curr_out));
        if(!b){
            printf("error write \n");
            break;
        }
    }
}

int main() {
  
    prog_data = new data_t();
    prog_data->main_portno = 0;
    prog_data->main_socket = 0;
    prog_data->base_dir = "";
    prog_data->users = vector<user_t *>();
    parse_grass(prog_data);
    int err = 0;
    err = init_server(prog_data);

    if(err){
        printf("Error : %d \n",err);
        return err;
    }
    accept_connections(prog_data);
    return 0;

}
