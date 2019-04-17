#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/sendfile.h>

#include "error.h"
#include "grass.h"

#define CHUNK_SIZE 256




int setup_ftp_connection_server(connection_t* client){
    int sockfd = -1; 
    struct sockaddr_in serv_addr;
    // open the server's socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        printf("ERROR opening socket in ftp_connection \n");
        return ERROR_SOCKET;
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
        return ERROR_NETWORK;
    }
    int error = 0;
    socklen_t len;
    error = getsockname(sockfd, (struct sockaddr *)&serv_addr,&len);
    if(error < 0){
        printf("Error : getsockname \n");
        return ERROR_NETWORK;
    }
    printf("portno : %d \n",serv_addr.sin_port);
    client->ftp_port = serv_addr.sin_port;
    return 0;
}


int file_send(int sock,int fd,size_t size){
    int sent = 0;
    int total_sent = 0;
    int remaining = size;
    off_t offset = 0;
    while( remaining > 0 &&
    ((sent = sendfile(sock,fd,&offset,remaining)) > 0)){
        printf("sent %d bytes of data \n",sent);
        remaining -= sent;
        total_sent += sent;
    }
    if(sent < 0){
        printf("Error : sendfile() \n");
        return ERROR_SEND;
    }
    return total_sent;
}

int file_recv(int sock,int fd, size_t size){
    int rec = 0;
    int total_recv = 0;
    int error = 0;
    size_t bufsize = CHUNK_SIZE > size ? size : CHUNK_SIZE;
    int remaining = size;
    off_t offset = 0;
    char buffer[bufsize];
    while((rec = recv(sock,buffer,bufsize,0)) > 0){
        printf("recv %d bytes of data \n",rec); 
        remaining -= rec;
        total_recv += rec;
        if(total_recv > size){
            printf("Error : file receive is too large \n");
            return ERROR_FILESIZE;
        }
        error = write(fd,buffer,rec);
        if(error < 0){
            printf("Error: while writing to file \n");
            return ERROR_IO;
        }        
    }
    if(rec < 0){
        printf("Error recv() \n");
        return ERROR_RECV;
    }
    return total_recv;
}