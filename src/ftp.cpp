#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "error.h"
#include "grass.h"




int setup_ftp_connection(connection_t* client){
    int sockfd = -1; 
    struct sockaddr_in serv_addr;
    // open the server's socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        printf("ERROR opening socket in ftp_connection \n");
    }
    client->ftp_socket = sockfd;
    // initialize the server's address sructure
    bzero((char*) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = 0;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // bind the server's address and port to the socket
    if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR on binding\n");
        return ERROR_BIND;
    }
    if(listen(sockfd,1) != 0){
        printf("ERROR listening failed\n");
        return -1;
    }
    return 0;

}