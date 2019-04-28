
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

using namespace std;

static int sock=0;
char cwd[MAX_PATH_SIZE];
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

int init(int& sock, int& portno,char** argv,ftp_data_t* ftp) {
    int error = 0;
    //initialise ctrl-c handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ctrl_c;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("[%s]\n",cwd);
    }
    //initialise connection
    portno = atoi(argv[2]);
    error = pthread_mutex_init(&(ftp->clean_lock),NULL);
    if(error){
        printf("Error Init ftp mutex \n");
        return error;
    }

    error = setup_client_co(argv[1],portno,&sock);
    if(error < 0){
        printf("Error setting up client \n");
        return error;
    }
    strncpy(ftp->ip,argv[1],IP_ADDRESS_MAX_LENGTH);
    ftp->ftp_user = FTP_CLIENT;    
    ftp->main_socket = sock;
    ftp->using_ftp = false;
    ftp->file_size = 0;
    return 0;
}

bool chat(int sock, char* buffer,ftp_data_t* ftp) {
    int n;
    char splitted_input[MAX_TOKENS][MAX_ARG_SIZE];
    char splitted_response[MAX_TOKENS][MAX_ARG_SIZE];
    int num_tokens;
     //actual chat
    while(!feof(stdin) && !ferror(stdin)) {
        cout << ">> ";
        bzero(buffer,256);
        if(feof(stdin) || ferror(stdin)) return false;
        fgets(buffer,255,stdin);
        for(int i = 0; i < MAX_TOKENS; i++){
            memset(&(splitted_input[i]),0,MAX_ARG_SIZE);
            memset(&(splitted_response[i]),0,MAX_ARG_SIZE);
        }
        //num_tokens = tokenize_cmd(buffer,splitted_input);
        if(strcmp("put",splitted_input[0]) == 0){
            if(num_tokens > 2){
                //strncpy(ftp->)
            }
        }
        n = write(sock,buffer,strlen(buffer));
        if (n < 0) 
            cout <<"ERROR writing to socket" << endl;
        bzero(buffer,BUFFER_READ_MAX_SIZE);
        n = read(sock,buffer,BUFFER_READ_MAX_SIZE);
        //num_tokens = tokenize_cmd(buffer,splitted_response);
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

int main(int argc, char **argv){
    if(argc != 3){
        printf("Error : Wrong Parameters\n");
        return 0;
    }
    ftp_data_t ftp_data;
    memset(&ftp_data,0,sizeof(ftp_data_t));
    // TODO:
    // Make a short REPL to send commands to the server
    // Make sure to also handle the special cases of a get and put command
    int portno=0;
    if(init(sock,portno,argv,&ftp_data)) return 0;

    char buffer[1024] = {0};

    bool running = true;
    while(running) {
        running = chat(sock, buffer,&ftp_data);
    }   
    close(sock); 

    return 0; 


}
