#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/sendfile.h>

#include "ftp.h"
#include "error.h"
#include "grass.h"

#define CHUNK_SIZE 256
#define MAX_CO_FTP 1

int setup_ftp_connection_server(connection_t* client){
    int error = 0;
    int portno = 0;
    int sock = -1;
    // setup server socket on random available port and update port number 
    error = setup_server_co(&portno,&sock,true,MAX_CO_FTP);
    if(error){
        printf("Error setup_server_co()\n");
        return error;
    }
    client->ftp_data.ftp_port = portno;
    client->ftp_data.ftp_socket = sock;
    client->ftp_data.using_ftp = true;
    return 0;
}

void check_ftp(ftp_data_t * ftp){
    pthread_mutex_lock(&(ftp->clean_lock));
    if(ftp->using_ftp == true){
        printf("Thread is already used, killing and freeing \n");
        stop_ftp_thread(ftp);
    }
    pthread_mutex_unlock(&(ftp->clean_lock));
}


void stop_ftp_thread(ftp_data_t* ftp){
    pthread_kill(ftp->ftp_id,SIGTERM);
    pthread_join(ftp->ftp_id,NULL);
    if(ftp->ftp_file != NULL){
        printf("closing file in stop_ftp_thread \n");
        fclose(ftp->ftp_file);
        ftp->ftp_file = NULL;
    }
    bzero(ftp->filepath,MAX_ROOT_PATH + MAX_PATH_SIZE);
    if(ftp->ftp_socket > 1){
        close(ftp->ftp_socket);
    }  
    ftp->using_ftp = false;
    ftp->file_size = 0;
    ftp->ftp_port = -1;
}


void ftp_end(ftp_data_t * ftp){
    pthread_mutex_lock(&(ftp->clean_lock));
    if(ftp->ftp_file != NULL){
        fclose(ftp->ftp_file);
        ftp->ftp_file = NULL;
    }
    bzero(ftp->filepath,MAX_ROOT_PATH + MAX_PATH_SIZE);
    if(ftp->ftp_socket > 1){
        close(ftp->ftp_socket);
    }
    ftp->using_ftp = false;
    ftp->file_size = 0;
    ftp->ftp_port = -1;
    pthread_mutex_unlock(&(ftp->clean_lock));
    pthread_exit((void *)0);
}

void *ftp_subthread(void* ptr){
    int new_sockfd;
    int error;
    ftp_data_t * ftp = (ftp_data_t*) ptr;
    if(ftp->ftp_type == FTP_RECV){
        ftp->ftp_file = fopen(ftp->filepath,"wb");
        if(ftp->ftp_file == NULL){
            printf("Error Opening file, thread_exiting \n");
            ftp_end(ftp);
        }
    }else{
        ftp->ftp_file = fopen(ftp->filepath,"rb");
        if(ftp->ftp_file == NULL){
            printf("Error Opening file, thread_exiting \n");
            ftp_end(ftp);
        }
    }
    int fd = fileno(ftp->ftp_file);
    if(fd < 0){
        printf("Error getting the file descriptor \n");    
        ftp_end(ftp);
    }
    if(ftp->ftp_user == FTP_SERVER){
        struct sockaddr_in cli_addr;
        socklen_t clilen;
        clilen = sizeof(cli_addr); 
        new_sockfd = accept(ftp->ftp_socket,(struct sockaddr *) &cli_addr,&clilen);
        if(new_sockfd < 0){
            printf("Error : FTP thread, accept() failed \n");
            ftp_end(ftp);
        }
        do_ftp(ftp,new_sockfd,fd);
    }else{
        error = setup_client_co(ftp->ip,ftp->ftp_port,&new_sockfd);
        if(error){
            printf("Error setting connection to server \n");
            ftp_end(ftp);
        }
        do_ftp(ftp,new_sockfd,fd);
    }
}

void do_ftp(ftp_data_t* ftp,int sockfd,int fd){
    if(ftp->ftp_type == FTP_RECV){
        int error = file_recv(sockfd,fd,ftp->file_size);
        if(error == ERROR_FILESIZE){
        char err_msg[] = "Error:  file transfer failed.";
        printf("error filesize \n");
        write(ftp->main_socket,err_msg,strlen(err_msg)); 
        }
        if(error == ERROR_NETWORK){
            printf("Error : file reception failed \n");
        }
        if(error >= 0){
            printf("received %d bytes of data \n",error);
        }     
    }else{
        int error = file_send(sockfd,fd,ftp->file_size);
        if(error != ftp->file_size){
            printf("Error file send failed or did not send required amount \n");
        }
        if(error >= 0){
            printf("sent %d bytes of data \n",error);
        }       
    }
    ftp_end(ftp);
}




int setup_ftp_connection_client(){
    
}

int setup_server_co(int * portno,int * sock,bool random_port,unsigned int max_co){
    int sockfd = -1; 
    struct sockaddr_in serv_addr;
    // open the server's socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        printf("ERROR opening socket in ftp_connection \n");
        return ERROR_SOCKET;
    }
    *sock =  sockfd;
    // initialize the server's address sructure
    bzero((char*) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    if(random_port){
        serv_addr.sin_port = 0;
    }else{
        serv_addr.sin_port = htons(*portno);
    }
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // bind the server's address and port to the socket
    if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR on binding\n");
        return ERROR_BIND;
    }
    if(listen(sockfd,max_co) != 0){
        printf("ERROR listening failed\n");
        return ERROR_NETWORK;
    }
    int error = 0;
    socklen_t len = sizeof(struct sockaddr);
    error = getsockname(sockfd, (struct sockaddr *)&serv_addr,&len);
    if(error < 0){
        printf("Error : getsockname \n");
        return ERROR_NETWORK;
    }
    *portno = ntohs(serv_addr.sin_port);
    printf("Port number is : %d \n",*portno);
    return 0;
}

int setup_client_co(char * ip,int portno,int* sock){
    struct sockaddr_in serv_addr; 
    int sockfd = -1;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
        printf("\n Socket creation error \n"); 
        return ERROR_SOCKET; 
    } 
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(portno); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0){ 
        printf("\nInvalid address/ Address not supported \n"); 
        return ERROR_NETWORK; 
    } 
   
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
        printf("Connection Failed \n"); 
        return ERROR_NETWORK; 
    }
    *sock = sockfd;
    return 0;
}


int file_send(int sock,int fd,size_t size){
    int sent = 0;
    int total_sent = 0;
    int remaining = size;
    off_t offset = 0;
    while( remaining > 0 &&
    ((sent = sendfile(sock,fd,&offset,remaining)) > 0)){
        remaining -= sent;
        total_sent += sent;
    }
    close(sock);
    if(sent < 0){
        printf("Error : sendfile() \n");
        return ERROR_SEND;
    }
    return total_sent;
}

int file_recv(int sock,int fd, size_t size){
    size_t rec = 0;
    size_t total_recv = 0;
    int error = 0;
    size_t bufsize = CHUNK_SIZE > size ? size : CHUNK_SIZE;
    int remaining = size;
    char buffer[bufsize];
    while((rec = recv(sock,buffer,bufsize,0)) > 0){
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
        if(total_recv == size){
            break;
        }         
    }
    close(sock);
    if(rec < 0){
        printf("Error recv() \n");
        return ERROR_RECV;
    }
    if(total_recv != size){
        printf("error file size \n");
        return ERROR_FILESIZE;
    }
    return total_recv;
}