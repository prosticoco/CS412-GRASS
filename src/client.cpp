
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include "ftp.h"
#include "grass.h" 
#include "error.h"

using namespace std;

static int sock=0;
int BUFFER_READ_MAX_SIZE = 1024;


void error(char *msg)
{
    perror(msg);
    exit(0);
}


void ctrl_c(int socket){
    cout << endl << "Stopping connection and exiting" << endl; 

    char buff[5] = "exit";
    write(sock, buff, 5); 

    close(sock);
    exit(0);
}

int init(int& sock, int& portno,char** argv) {

    //initialise ctrl-c handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ctrl_c;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    //initialise connection
    portno = atoi(argv[2]);
    //char ip_buffer[20];
    //strncpy(ip_buffer, argv[1], 18);
    int error = 0;
    error = setup_client_co(argv[1],portno,&sock);
    if(error < 0){
        printf("Error setting up client \n");
        return error;
    }
    return 0;
    /**
    struct sockaddr_in serv_addr; 

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(portno); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip_buffer, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("Connection Failed \n"); 
        return -1; 
    }
    **/
}

bool chat(int sock, char* buffer) {
    int n;
     //actual chat
    while(!feof(stdin) && !ferror(stdin)) {
        cout << ">> ";
        bzero(buffer,256);
        if(feof(stdin) || ferror(stdin)) return false;
        fgets(buffer,255,stdin);
        n = write(sock,buffer,strlen(buffer));
        if (n < 0) 
            cout <<"ERROR writing to socket" << endl;
        bzero(buffer,BUFFER_READ_MAX_SIZE);
        n = read(sock,buffer,BUFFER_READ_MAX_SIZE);
        if (n < 0) {
            error("ERROR reading from socket");
        }
        if(n == 0) {
            printf("Good bye, booooy\n");
            return false;
        }
        printf("%s\n",buffer);
        
    }

    return false;
}

/*
 * Send a file to the server as its own thread
 *
 * fp: file descriptor of file to send
 * d_port: destination port
 */
void send_file(int fp, int d_port) {
    // TODO
}

/*
 * Recv a file from the server as its own thread
 *
 * fp: file descriptor of file to save to.
 * d_port: destination port
 * size: the size (in bytes) of the file to recv
 */
void recv_file(int fp, int d_port, int size) {
    // TODO
}

/*
 * search all files in the current directory
 * and its subdirectory for the pattern
 *
 * pattern: an extended regular expressions.
 */
void search(char *pattern) {
    // TODO
}

int main(int argc, char **argv) {
    // TODO:
    // Make a short REPL to send commands to the server
    // Make sure to also handle the special cases of a get and put command
    int portno=0;
    if(init(sock,portno,argv)) return 0;

    char buffer[1024] = {0};

    bool running = true;
    while(running ) {
        running = chat(sock, buffer);
    }   
    close(sock); 

    return 0; 


}
