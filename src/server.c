#include <grass.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_MAX_SIZE 256

static struct User **userlist;
static int numUsers;
static struct Command **cmdlist;
static int numCmds;
char port[7] = "31337";

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
void parse_grass() {
}

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
 * @brief Test function for starting the project, once connection is accepted
 * chat with the client YOHO
 * 
 * @param sockfd the socket for chatting
 */
void test_function(int sockfd){
    char buffer[BUFFER_MAX_SIZE];
    int n;

    while(1){
        printf("Reading from socket \n");
        bzero(buffer,BUFFER_MAX_SIZE);
        read(sockfd,buffer,sizeof(buffer));
        printf("From Client : %s \n To Client : ",buffer);
        bzero(buffer,BUFFER_MAX_SIZE);
        n = 0;
        while((buffer[n++] = getchar()) != '\n');

        write(sockfd,buffer,sizeof(buffer));

        if(strncmp("exit",buffer,4) == 0){
            printf("Server Exit ... \n");
            break;
        }


    }
}

int main() {

    // TODO:
    // Parse the grass.conf file
    parse_grass();
    // Listen to the port and handle each connection

    // initial variables for connection information (TO BE CHANGED)
    int sockfd,newsockfd;
    int portno = 31337;
    int clilen;
    int n;
    char buffer[BUFFER_MAX_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    // open the server's socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        error("ERROR opening socket \n");
    }

    // initialize the server's address sructure
    bzero((char*) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // bin the server's address and port to the socket
    if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding\n");
    }

    // listen to 5 maximum connexions
    if(listen(sockfd,5) != 0){
        error("ERROR listening failed\n");
    }

    printf("Waiting to connect to a client \n");

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
    if(newsockfd < 0){
        error("ERROR on accept\n");
    }
    printf("Accepted connection \n");

    test_function(newsockfd);

    close(sockfd);

    return 0;





}
