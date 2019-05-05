/**
 * @file ftp.h
 * @author Adrien Prost and Rodrigo Granja
 * @brief contains most ftp-related and other network related low level functions
 * @version 0.1
 * @date 2019-05-05
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "grass.h"

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
 * @brief writes contents of a file on a socket/ sends a file to socket
 * 
 * @param sock the socket file descriptor for which we send the file
 * @param fd file descriptor of the file
 * @param size the expected file size
 * @return int 0 upon success, negative error code on network error or size mismatch
 */
int file_send(int sock,int fd,size_t size);


/**
 * @brief receives a file for which the contents are read from a socket
 * and writes the contents of the file  
 * 
 * @param sock the socket on which the function retrieves the file
 * @param fd file descriptor of the new file (on which we write the contents)
 * @param size the expected size of the file
 * @return int 0 upon success, negative error code on network error or size mismatch
 */
int file_recv(int sock,int fd, size_t size);

/**
 * @brief checks if the client already has an active ftp connection, if not, calls connect_ftp_server
 * (explained below)
 * to setup a new ftp connection with a random port 
 * code is protected by a mutex lock to avoid race conditions on ftp metadata
 * 
 * @param client pointer to the client connection metadata
 * @return int 0 upon success, negative error code upon network error
 */
int setup_ftp(connection_t * client);

/**
 * @brief Checks if the caller is the client or the server,
 * if client : calls connect_ftp_client
 * if server : calls connect_ftp_server
 * if these functions return an error, this function calls check_ftp to end the ftp connection
 * in a clean way
 * @param ftp  ftp metadata
 * @param type FTP_SERVER or FTP_CLIENT constants defined in grass.h to specify the caller
 */
void ftp_connect(ftp_data_t * ftp,int type);

/**
 * @brief calls accept() to wait for the client to connect for file transfer (called only by the server)
 * ftp metadata is protected with a mutex lock
 * @param ftp  ftp metadata
 * @return int 0 upon success, negative error code upon network error
 */
int connect_ftp_server(ftp_data_t* ftp);

/**
 * @brief calls connect to connect the client to the server for file transfer (called only by the client)
 * ftp metadata is protected with a mutex lock
 * 
 * @param ftp ftp metadata
 * @return int 0 upon success, negative error code upon network error 
 */
int connect_ftp_client(ftp_data_t * ftp);

/**
 * @brief initializes the fields of the ftp metadata
 * 
 * @param ftp the ftp metadata to initialize
 */
void init_ftp_fields(ftp_data_t* ftp);

/**
 * @brief Main function to stop ftp connection closes all file descriptors and sockets
 * and updates the connection's ftp metadata
 * 
 * @param ftp the ftp metadata of the connection
 * @param type ftp type can either be SEND or RECV (defined in grass.h)
 * @param stop_threads specifies if the caller should stop any active thread Sending or Receiving
 * @param exit specifies if the caller should exit (pthread_exit) after cleaning
 */
void ftp_clean(ftp_data_t* ftp,int type,bool stop_threads,bool exit);

/**
 * @brief checks if there are any active threads receiving or sending (according to the 
 * type specifie), if so it will call ftp_clean with the same parameters
 * function is locked with a mutex in order to ensure there are no dataraces on 
 * the ftp metadata
 * @param ftp ftp metadata
 * @param type ftp type can either be SEND or RECV (defined in grass.h)
 * @param stop_threads specifies if the caller should stop any active thread Sending or Receiving
 * @param exit  specifies if the caller should exit (pthread_exit) after cleaning
 */
void check_ftp(ftp_data_t * ftp,int type,bool stop_threads,bool exit);

/**
 * @brief thread function which takes care of sending a file
 * calls check_ftp at the end or if any error occurs
 * 
 * @param ptr ftp metadata (void pointer is cast to ftp_data_t *)
 * @return void* exits at the end
 */
void *ftp_thread_send(void* ptr);

/**
 * @brief thread function which takes care of handling the reception of a file
 * calls check_ftp at the end or if any error occurs
 * 
 * @param ptr ftp metadata (void pointer is cast to ftp_data_t *)
 * @return void* exits at the end
 */
void *ftp_thread_recv(void* ptr);

/**
 * @brief performs ftp, function is called to either send or receive a file on
 * a given socket 
 * 
 * @param ftp the ftp metadata
 * @param sockfd the socket on which to send/receive the file
 * @param fd the file descriptor for handling the file
 */
void do_ftp(ftp_data_t* ftp,int sockfd,int fd);