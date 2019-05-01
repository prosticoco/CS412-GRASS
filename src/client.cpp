
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

#define END_CONNECTION 1
#define END_ERROR -1
using namespace std;
int BUFFER_READ_MAX_SIZE = 1024;
client_t client_data;




void *client_reader(void* ptr){
    int n = 0;
    int num_tokens = 0;
    client_t * client = (client_t*) ptr;
    char buffer[MAX_OUTPUT_SIZE];
    while(1){
        bzero(buffer,MAX_OUTPUT_SIZE);
        n = read(client->ftp_data.main_socket,buffer,MAX_OUTPUT_SIZE);
        if (n < 0) {
            printf("ERROR reading from socket\n");
            stop_and_clean(END_ERROR);
        }
        if(n == 0) {
            printf("End of connection \n");
            stop_and_clean(END_CONNECTION);
        }
        printf("%s\n",buffer);
        num_tokens = check_response(buffer,MAX_OUTPUT_SIZE,client);
    }
    stop_and_clean(END_ERROR);
}

void *client_writer(void* ptr){
    int n = 0;
    client_t * client = (client_t*) ptr;
    char buffer[MAX_INPUT_SIZE];
    int num_tokens = 0;
    while(!ferror(stdin) && !feof(stdin)){  
        bzero(buffer,MAX_INPUT_SIZE);
        fgets(buffer,MAX_INPUT_SIZE -1,stdin);
        num_tokens = check_input(buffer,MAX_INPUT_SIZE,client);
        if(!num_tokens){
            continue;
        }
        n = write(client->ftp_data.main_socket,buffer,strlen(buffer));
        if (n < 0){
            cout <<"ERROR writing to socket" << endl;
            stop_and_clean(END_ERROR);
        }    
    }
    stop_and_clean(END_ERROR);
}

int check_input(char* input,size_t size,client_t* client){
    char splitted_input[MAX_TOKENS][MAX_ARG_SIZE];
    for(int i = 0; i < MAX_TOKENS; i++){
            memset(&(splitted_input[i]),0,MAX_ARG_SIZE);
        }
    int num_tokens = 0;
    num_tokens = tokenize_cmd(input,splitted_input);
    if(num_tokens > 1){
        if(!strcmp("put",splitted_input[0])){           
            if(num_tokens > 2){
                check_ftp(&(client->ftp_data));
                strncpy(client->ftp_data.filepath,client->cwd,MAX_ROOT_PATH);
                strcat(client->ftp_data.filepath,"/");
                strncat(client->ftp_data.filepath,splitted_input[1],MAX_FILENAME_SIZE);
                client->ftp_data.file_size = atoi(splitted_input[2]);
                client->ftp_data.using_ftp = true;             
            }
        }
        if(!strcmp("get",splitted_input[0])){

        }

    }
    return num_tokens;
}

int check_response(char* response,size_t size,client_t* client){
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
                client->ftp_data.ftp_port = atoi(splitted_response[2]);
                client->ftp_data.ftp_type = FTP_SEND;
                pthread_mutex_unlock(&(client->ftp_data.clean_lock));
                pthread_create(&(client->ftp_data.ftp_id),NULL,ftp_subthread,(void *) &(client->ftp_data));
            }
        }
    }
    return num_tokens;
}

void stop_and_clean(int signum){
    if(signum == END_ERROR){
        printf("Error reader/writer\n");
    }
    if(signum == END_CONNECTION){
        printf("Lost connection to server \n");
    }
    if(pthread_self() == client_data.main){
        printf("Stopping reader\n");
        pthread_kill(client_data.reader, SIGTERM);
        pthread_join(client_data.reader,NULL);
        printf("Stopping write\n");
        pthread_kill(client_data.writer, SIGTERM);
        pthread_join(client_data.writer,NULL);
        printf("killing any leftover ftp thread\n");
        check_ftp(&(client_data.ftp_data));
    }else{
        pthread_kill(client_data.main,SIGTERM);
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
        printf("[%s]\n",client->cwd);
    }
    //initialise connection
    client->portno = atoi(argv[2]);
    client->main = pthread_self();
    printf("Client main is : [%lu] \n",client->main);
    pthread_mutex_init(&(client->ftp_data.clean_lock),NULL);
    pthread_mutex_init(&(client->lock),NULL);
    error = setup_client_co(argv[1],client->portno,&sock);
    if(error < 0){
        printf("Error setting up client \n");
        return error;
    }
    strncpy(client->ftp_data.ip,argv[1],IP_ADDRESS_MAX_LENGTH);
    client->ftp_data.ftp_user = FTP_CLIENT;    
    client->ftp_data.main_socket = sock;
    client->ftp_data.using_ftp = false;
    client->ftp_data.file_size = 0;
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
