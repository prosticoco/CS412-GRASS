#include <grass.h> 
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(char *msg)
{
    perror(msg);
    exit(0);
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

    int sockfd, portno, n;
    int valread;

    //check that user input for port num is actually a number

    struct sockaddr_in serv_addr;

    char buffer[256];
    if (argc < 3) {
        strncpy(buffer, argv[1], 17);
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        memset(buffer, 0, sizeof(buffer)); 
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    // assign IP, PORT 
    strncpy(buffer, argv[1], 17);
    printf(buffer);
    printf("\n");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(buffer); 
    serv_addr.sin_port = htons(portno); 
  
    // connect the client socket to server socket 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
            printf("\nConnection Failed \n"); 
            return -1; 
    } 
    

    //actual chat
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    
    return 0;



}
