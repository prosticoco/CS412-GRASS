#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "exit.h"
#include "utils.h"

data_t* prog_data = NULL;
/**
 * @brief this stop function is called whenever the user shuts down the server and will 
 * clean the memory and threads before exiting.
 * 
 * @param signum signal number which triggered the function
 */
void stop(int signum) {
    pthread_t curr_tid = pthread_self();
    if(curr_tid != prog_data->main_tid){
        thread_end();
    }
    printf("Freeing data \n");
	//reset signal handlers to default
    if(prog_data == NULL){
        printf("prog_data is null \n");
        exit(0);
    }
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
    connection_t * tmp;
    for(auto c : prog_data->connections){
        pthread_join(c->tid,NULL);
        tmp = c;
        free(tmp->username);
        close(tmp->connection_socket);
        free(tmp);
    }
    prog_data->connections.clear();
    for(auto u : prog_data->users){
        free(u->uname);
        free(u->pass);
        free(u);
    }
    prog_data->users.clear();
    close(prog_data->main_socket);
    free(prog_data);
    exit(signum);
}


/**
 * @brief Function called by thread when the client disconnects, 
 * it safely cleans the memory space used for this client. as well as closing the socket
 * 
 * @param c 
 * @return int 
 */
int thread_cleanup(connection_t* c){
    printf("Freeing data of user : [%s] with tid : [%lu] \n",c->username,c->tid);
    remove_connection(c->server_data,c);
    free(c->username);
    close(c->connection_socket);
    free(c);
    printf("Tread exiting...\n");
    pthread_exit((void *) 0);
}

/**
 * @brief This function is called whenever the user shuts down the server
 * it tells the running thread to stop running. The stop function above will take
 * care of whatever is left to cleanup
 * 
 * @param signum 
 * @return int 
 */
void thread_end(){
    printf("Thread [%lu] exiting... \n",pthread_self());
    pthread_exit((void *) 0);
}
