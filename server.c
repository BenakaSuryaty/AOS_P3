//standard imports
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>
#include<glib.h>

#include "queue.h"

//for socket api and functionality
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>


//header to store address info
#include<netinet/in.h>

//NOTE: globals

#define ERROR (-1)
#define SERVER_STATUS 0 //0: accepting connections(storing in mem); 1: stopped connection(writing to storage) 
#define SERVER_PORT 4000 //[ ]: change to dc machine's available port
#define SERVER_IP "127.0.0.1"//[ ]: change to dc machine's IP
#define SERVER_BACKLOG 6
#define THREAD_POOL_SIZE 10

//struct def to store address info
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

//mutex and condition for the threads
pthread_mutex_t eventQueue_mutex;
pthread_cond_t eventQueue_cond;

//event queue



//NOTE: helper funcitons

//func to check the errors
int check(int exp, const char *msg){
    if(exp == ERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}



//TODO: Add helper functions for the main logic

int main(){
    //hash to store key value pair
    GHashTable* hash = g_hash_table_new(g_str_hash, g_str_equal);


    //thread def
    pthread_t th[THREAD_POOL_SIZE];
    pthread_mutex_init(&eventQueue_mutex, NULL);
    pthread_cond_init(&eventQueue_cond, NULL);
  /*
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&th[i], NULL, , NULL) != 0) { //[ ]: fix according to the helper functions.
            perror("Failed to create the thread");
        }
    }
*/

    //socket creation
    int server_sock, client_sock, addr_size;
    SA_IN server_addr, client_addr;

    check((server_sock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket.");

    //initializing addr struct
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    check((bind(server_sock,(struct sockaddr*) &server_addr, sizeof(server_addr))),"Failed to bind.");

    check((listen(server_sock,SERVER_BACKLOG)),"Listen failed.");

    while (true)
    {
        //TODO: main logic
    }
    
    //joining threads and destroying pthread vars
        for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join the thread");
        }
    }
    pthread_mutex_destroy(&eventQueue_mutex);
    pthread_cond_destroy(&eventQueue_cond);
   return 0; 
}