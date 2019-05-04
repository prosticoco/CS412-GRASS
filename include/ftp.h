
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

/**
 * @brief 
 * 
 * @param sock 
 * @param fd 
 * @param size 
 * @return int 
 */
int file_recv(int sock,int fd, size_t size);

int setup_ftp(connection_t * client);

void ftp_connect(ftp_data_t * ftp,int type);

int connect_ftp_server(ftp_data_t* ftp);

int connect_ftp_client(ftp_data_t * ftp);

void init_ftp_fields(ftp_data_t* ftp);

void ftp_clean(ftp_data_t* ftp,int type,bool stop_threads,bool exit);

void *ftp_thread_send(void* ptr);

void *ftp_thread_recv(void* ptr);

void ftp_end(ftp_data_t * ftp,bool exit);

void check_ftp(ftp_data_t * ftp,int type,bool stop_threads,bool exit);

void do_ftp(ftp_data_t* ftp,int sockfd,int fd);