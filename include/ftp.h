
#include "grass.h"

int setup_ftp_connection_server(connection_t* client);

/**
 * @brief creates a new connection as a server :
 * initializes a socket
 * binds the localhost address and specified port to the socket
 * 
 * @param portno reference specified port number (doesnt need to be initialized if random)
 * @param sock reference to the socket file descriptor
 * @param random_port boolean to specify if the port should be random or not
 * @param max_co maximum number of connections allowed simultaneously on this socket
 * @return int returns 0 upon success, negative code number if error raised
 */
int setup_server_co(int * portno,int * sock,bool random_port,unsigned int max_co);

/**
 * @brief creates a new connection as a client:
 * initializes a socket
 * connects to a server for which the ip address and port number is specified in the parameters
 * 
 * @param ip ip address of server as a string
 * @param portno port number of server
 * @param sock reference to the socket file descriptor
 * @return int 0 upon success, negative code number if error raised
 */
int setup_client_co(char * ip,int portno,int* sock);

/**
 * @brief 
 * 
 * @param sock 
 * @param fd 
 * @param size 
 * @return int 
 */
int file_send(int sock,int fd,size_t size);

int file_recv(int sock,int fd, size_t size);


int stop_ftp_thread(connection_t* client);

void *ftp_subthread(void* ptr);

void ftp_end(ftp_data_t * ftp);

void do_ftp(ftp_data_t* ftp,int sockfd,int fd);