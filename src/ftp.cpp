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
#include "utils.h"
#include "grass.h"

void init_ftp_fields(ftp_data_t* ftp){
    bzero(ftp,sizeof(ftp_data_t));
    ftp->port_open = false;
    ftp->sending = false;
    ftp->receiving = false;
    ftp->ftp_file_recv = NULL;
    ftp->ftp_file_send = NULL;
    ftp->file_size_recv = 0;
    ftp->file_size_send = 0;
    ftp->ftp_socket = -1;
    ftp->ftp_port = -1;
    ftp->file_transfer_socket = -1;
}

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
    return 0;
}

void check_ftp(ftp_data_t * ftp,int type,bool stop_threads,bool exit){ 
    pthread_mutex_lock(&(ftp->clean_lock));
    if(type == SEND){
        if(ftp->sending == true){
            ftp_clean(ftp,type,stop_threads,exit);
        }
    }else{
        if(ftp->receiving == true){
            ftp_clean(ftp,type,stop_threads,exit);
        }
    }
    pthread_mutex_unlock(&(ftp->clean_lock));
}

int setup_ftp(connection_t * client){
    int error = 0;
    pthread_mutex_lock(&(client->ftp_data.clean_lock));
    if(!(client->ftp_data.port_open)){
        error = setup_ftp_connection_server(client);
        if(error){
            pthread_mutex_unlock(&(client->ftp_data.clean_lock));
            return error;
        }
    }
    pthread_mutex_unlock(&(client->ftp_data.clean_lock));
    return error;
}


void ftp_clean(ftp_data_t* ftp,int type,bool stop_threads,bool exit){
    if(type == SEND){
        if(stop_threads){
            pthread_cancel(ftp->send_id);
            pthread_join(ftp->send_id,NULL);
        }        
        if(ftp->ftp_file_send != NULL){
            printf("Closing file used for put\n");
            fclose(ftp->ftp_file_send);
            ftp->ftp_file_send = NULL;
        }
        bzero(ftp->filepath_send,MAX_ROOT_PATH + MAX_PATH_SIZE);
        ftp->sending = false;
        ftp->file_size_send = 0;
    }else{
        if(stop_threads){
            pthread_cancel(ftp->recv_id);
            pthread_join(ftp->recv_id,NULL);
        }
        if(ftp->ftp_file_recv != NULL){
            printf("Closing file used for put\n");
            fclose(ftp->ftp_file_recv);
            ftp->ftp_file_recv = NULL;
        }
        bzero(ftp->filepath_recv,MAX_ROOT_PATH + MAX_PATH_SIZE);
        ftp->receiving = false;
        ftp->file_size_recv = 0;
    }
    if((ftp->receiving == false) && (ftp->sending == false)){
        if(ftp->ftp_socket > 1){
            close(ftp->ftp_socket);        
        }
        ftp->ftp_port = -1;
        if(ftp->file_transfer_socket > 1){
            close(ftp->file_transfer_socket);
        }
        ftp->file_transfer_socket = -1;
        ftp->port_open = false; 
    }
    if(exit){
        pthread_mutex_unlock(&(ftp->clean_lock));
        pthread_exit((void *) 0);
    }
}

int connect_ftp_server(ftp_data_t* ftp){
    int sock = 0;
    int ret = 0;
    pthread_mutex_lock(&(ftp->clean_lock));
    if(!ftp->port_open){
        struct sockaddr_in cli_addr;
        socklen_t clilen;
        clilen = sizeof(cli_addr); 
        sock = accept(ftp->ftp_socket,(struct sockaddr *) &cli_addr,&clilen);
        if(sock < 0){
            printf("Error on accept() \n");
            ret = ERROR_NETWORK;
        }else{
            ftp->file_transfer_socket = sock;
            ftp->port_open = true;
        }
    }
    pthread_mutex_unlock(&(ftp->clean_lock));
    return ret;
}

int connect_ftp_client(ftp_data_t * ftp){
    int ret = 0;
    pthread_mutex_lock(&(ftp->clean_lock));
    if(!ftp->port_open){
        ret = setup_client_co(ftp->ip,ftp->ftp_port,&(ftp->file_transfer_socket));
        if(!ret){
            ftp->port_open = true;
        }
    }
    pthread_mutex_unlock(&(ftp->clean_lock));
    return ret;
}


void *ftp_thread_recv(void* ptr){
    int error = 0;
    int fd;
    ftp_data_t * ftp = (ftp_data_t *) ptr;
    int type = RECV;        
    ftp_connect(ftp,type);
    ftp->ftp_file_recv = fopen(ftp->filepath_recv,"wb");
    if(ftp->ftp_file_recv == NULL){
        printf("Error opening file, thread exiting \n");
        check_ftp(ftp,type,false,true);
    }
    fd = fileno(ftp->ftp_file_recv);
    if(fd < 0){
        printf("Error getting the file descriptor \n");    
        check_ftp(ftp,type,false,true);
    }
    do_ftp(ftp,fd,type);
    check_ftp(ftp,type,false,true);
} 


void *ftp_thread_send(void* ptr){
    int error = 0;
    int fd;
    ftp_data_t * ftp = (ftp_data_t *) ptr;
    int type = SEND;    
    ftp_connect(ftp,type); 
    ftp->ftp_file_send = fopen(ftp->filepath_send,"rb");
    if(ftp->ftp_file_send == NULL){
        printf("Error opening file, thread exiting \n");
        check_ftp(ftp,type,false,true);
    }
    fd = fileno(ftp->ftp_file_send);
    if(fd < 0){
        printf("Error getting the file descriptor \n");    
        check_ftp(ftp,type,false,true);
    }
    do_ftp(ftp,fd,type);
    check_ftp(ftp,type,false,true);
}

void ftp_connect(ftp_data_t * ftp,int type){
    int error = 0;
    if(ftp->ftp_user == FTP_SERVER){
        error = connect_ftp_server(ftp);
        if(error < 0){
            printf("Error setting up ftp connection from server\n");
            check_ftp(ftp,type,false,true);
        }
    }else{
        error = connect_ftp_client(ftp);
        if(error){
            printf("Error setting up ftp connection from client \n");
            check_ftp(ftp,type,false,true);
        }
    }
}



void do_ftp(ftp_data_t* ftp,int fd,int type){
    printf("socket is : %d \n",ftp->file_transfer_socket);
    if(type == RECV){
        int error = file_recv(ftp->file_transfer_socket,fd,ftp->file_size_recv);
        if(error == ERROR_FILESIZE){
        char err_msg[] = "Error:  file transfer failed.";
        printf("error filesize for file : [%s] \n",ftp->filepath_recv);
        printf("Expected file size : %zu \n",ftp->file_size_recv);
        write(ftp->main_socket,err_msg,strlen(err_msg)); 
        }
        if(error == ERROR_NETWORK){
            printf("Error : file reception failed \n");
        }
        if(error >= 0){
            printf("received %d bytes of data \n",error);
        }     
    }else{
        int error = file_send(ftp->file_transfer_socket,fd,ftp->file_size_send);
        if(((size_t)error) != ftp->file_size_send){
            printf("Error file send failed or did not send required amount \n");
        }
        if(error >= 0){
            printf("sent %d bytes of data \n",error);
        }       
    }
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
            printf("Error : file receive is too large expected size : %zu,size received : %zu \n",size,total_recv);
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
    if(total_recv != size){
        printf("error file size \n");
        return ERROR_FILESIZE;
    }
    return total_recv;
}