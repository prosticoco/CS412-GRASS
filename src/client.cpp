
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
#include "cmd.h"
#include "utils.h"

#define END_CONNECTION 1
#define END_ERROR -1
using namespace std;
int BUFFER_READ_MAX_SIZE = 1024;
client_t client_data;
bool stop = false;




void *client_reader(void* ptr){
    int n = 0;
    client_t * client = (client_t*) ptr;
    char buffer[MAX_OUTPUT_SIZE];
    while(1){
        bzero(buffer,MAX_OUTPUT_SIZE);
        n = read(client->ftp_data.main_socket,buffer,MAX_OUTPUT_SIZE);
        
        if (n < 0) {
            printf("ERROR reading from socket\n");
            pthread_kill(client->main,SIGTERM);
            break;
        }
        if(n == 0) {
            printf("End of connection \n");
            pthread_kill(client->main,SIGTERM);
            break;
        }
        
        printf("%s\n",buffer);
        //print_ftp_fields(&(client->ftp_data));
        check_response(buffer,client);
        pthread_mutex_unlock(&(client->lock));       
    }
    return (void *) 0;
}

void *client_writer(void* ptr){
    int n = 0;
    client_t * client = (client_t*) ptr;
    char buffer[MAX_INPUT_SIZE];
    int num_tokens = 0;
    while(!ferror(stdin) && !feof(stdin)){ 
        bzero(buffer,MAX_INPUT_SIZE);      
        fgets(buffer,MAX_INPUT_SIZE -1,stdin);       
        num_tokens = check_input(buffer,client);
        if(!num_tokens){
            
            continue;
        }   
        pthread_mutex_lock(&(client->lock));
        n = write(client->ftp_data.main_socket,buffer,strlen(buffer));
        if (n < 0){
            cout <<"ERROR writing to socket" << endl;
            pthread_kill(client->main,SIGTERM);
            return (void *) 0;
        }
            
    }
    pthread_kill(client->main,SIGTERM);
    return (void *) 0;
}

int check_input(char* input,client_t* client){
    char splitted_input[MAX_TOKENS][MAX_ARG_SIZE];
    for(int i = 0; i < MAX_TOKENS; i++){
            memset(&(splitted_input[i]),0,MAX_ARG_SIZE);
        }
    int num_tokens = 0;
    num_tokens = tokenize_cmd(input,splitted_input);
    if(num_tokens > 1){
        if(!strcmp("put",splitted_input[0])){           
            if(num_tokens > 2){
                check_ftp(&(client->ftp_data),SEND,true,false);
                strncpy(client->ftp_data.filepath_send,client->cwd,MAX_ROOT_PATH);
                strcat(client->ftp_data.filepath_send,"/");
                strncat(client->ftp_data.filepath_send,splitted_input[1],MAX_FILENAME_SIZE);
                client->ftp_data.file_size_send = atoi(splitted_input[2]);            
            }
        }
        if(!strcmp("get",splitted_input[0])){
            if(num_tokens > 1){
                // check if ftp is already in use
                check_ftp(&(client->ftp_data),RECV,true,false);
                // update the file path
                strncpy(client->ftp_data.filepath_recv,client->cwd,MAX_ROOT_PATH);
                strcat(client->ftp_data.filepath_recv,"/");
                strncat(client->ftp_data.filepath_recv,splitted_input[1],MAX_FILENAME_SIZE);
            }

        }

    }
    return num_tokens;
}

int check_response(char* response,client_t* client){
    int num_tokens = 0;
    char splitted_response[MAX_TOKENS][MAX_ARG_SIZE];
    for(int i = 0; i < MAX_TOKENS; i++){
            memset(&(splitted_response[i]),0,MAX_ARG_SIZE);
        }
    num_tokens = tokenize_cmd(response,splitted_response);
    if(num_tokens > 1){
        if(!strcmp("put",splitted_response[0])){
            if(num_tokens > 2){
                pthread_mutex_lock(&(client->ftp_data.clean_lock));
                if(!(client->ftp_data.port_open)){
                    client->ftp_data.ftp_port = atoi(splitted_response[2]);
                }
                client->ftp_data.sending = true;
                pthread_mutex_unlock(&(client->ftp_data.clean_lock));
                pthread_create(&(client->ftp_data.send_id),NULL,ftp_thread_send,(void *) &(client->ftp_data));
            }
        }
        if(!strcmp("get",splitted_response[0])){
            // check response has enough tokens
            if(num_tokens > 4){
                fflush(stdout);
                // update atomically the fields related to ftp response
                pthread_mutex_lock(&(client->ftp_data.clean_lock));
                if(!(client->ftp_data.port_open)){
                    client->ftp_data.ftp_port = atoi(splitted_response[2]);        
                }
                client->ftp_data.file_size_recv = atoi(splitted_response[4]);
                //printf("size received : %zu \n",client->ftp_data.file_size_recv);
                client->ftp_data.receiving = true;
                pthread_mutex_unlock(&(client->ftp_data.clean_lock));
                // spawn the thread which will receive the file
                pthread_create(&(client->ftp_data.recv_id),NULL,ftp_thread_recv,(void *) &(client->ftp_data));
            }
        }
    }
    // return the number of tokens
    return num_tokens;
}

void stop_and_clean(int signum){  
    if(pthread_self() == client_data.main){
        if(signum == END_ERROR){
            printf("Error reader/writer\n");
        }
        if(signum == END_CONNECTION){
            printf("Lost connection to server \n");
        }
        pthread_cancel(client_data.reader);
        pthread_cancel(client_data.writer);
        check_ftp(&(client_data.ftp_data),SEND,true,false);
        check_ftp(&(client_data.ftp_data),RECV,true,false);
    }else{    
        pthread_exit((void *)0);  
    }
    cout << "Stopping connection and exiting" << endl; 
    char buff[] = "exit";
    write(client_data.ftp_data.main_socket,buff,strlen(buff));
    printf("wrote to server and closing socket now \n");
    close(client_data.ftp_data.main_socket);
    exit(0);
}
int init(client_t* client,char** argv) {
    signal(SIGTERM, stop_and_clean);
	signal(SIGINT, stop_and_clean);
    int error = 0;
    int sock = -1;
    memset(&client_data,0,sizeof(client_t));
    if (getcwd(client->cwd,MAX_ROOT_PATH)  != NULL) {
    }
    //initialise connection
    client->portno = atoi(argv[2]);
    client->main = pthread_self();
    pthread_mutex_init(&(client->ftp_data.clean_lock),NULL);
    pthread_mutex_init(&(client->lock),NULL);
    error = setup_client_co(argv[1],client->portno,&sock);
    if(error < 0){
        printf("Error setting up client \n");
        return error;
    }  
    init_ftp_fields(&(client->ftp_data));
    strncpy(client->ftp_data.ip,argv[1],IP_ADDRESS_MAX_LENGTH);
    client->ftp_data.ftp_user = FTP_CLIENT;    
    client->ftp_data.main_socket = sock;
    return 0;
}

int main(int argc, char **argv){
    if(argc != 3){
        printf("Error : Wrong Parameters\n");
        return 0;
    }
    if(init(&client_data,argv)) return 0;

    pthread_create(&(client_data.reader),NULL,client_reader,(void *)&client_data);
    pthread_create(&(client_data.writer),NULL,client_writer,(void *)&client_data);

    pthread_join(client_data.reader,NULL);
    pthread_join(client_data.writer,NULL);

    return 0; 


}
