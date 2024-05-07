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
#define THREAD_POOL_SIZE 15
#define MAX_HANDSHAKE_MSG_SIZE 1024

//struct def to store address info
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

//mutex and condition for the threads
pthread_mutex_t eventQueue_mutex;
pthread_cond_t eventQueue_cond;


//NOTE: helper funcitons


//event loop function to check for pending client in the queue
//[ ] complete this function
void * EventLoop(void * arg){
    while(true){
        int *pclient = dequeue();
        if(pclient != NULL){
            //some function to handle connection
            continue;
            /*handle client function*/
        }
    }
}

//[ ] edit this to match the ack/non-ack message structure
bool handshake_protocol(int client_sock, char* addr) {
    char server_msg[MAX_HANDSHAKE_MSG_SIZE];
    char client_msg[MAX_HANDSHAKE_MSG_SIZE];
    ssize_t bytes_received, bytes_sent;

    //handshake init
    printf("Handshake initiated.\n");

    // Construct the server hello message with the client's IP address
    char server_hello[MAX_HANDSHAKE_MSG_SIZE];
    snprintf(server_hello, MAX_HANDSHAKE_MSG_SIZE, "Hello client with IP: %s! Please send your handshake message.\n", addr);

    // Step 1: Server sends the handshake message
    bytes_sent = send(client_sock, server_hello, strlen(server_hello), 0);
    if (bytes_sent == -1) {
        perror("send");
        return false;
    }

    // Step 2: Client receives the handshake message
    bytes_received = recv(client_sock, client_msg, sizeof(client_msg), 0);
    if (bytes_received == -1) {
        perror("recv");
        return false;
    }
    client_msg[bytes_received] = '\0';
    printf("Client: %s\n", client_msg);

    // Step 3: Client sends its handshake response
    const char* client_response = "Received server hello. Handshake complete.\n";
    bytes_sent = send(client_sock, client_response, strlen(client_response), 0);
    if (bytes_sent == -1) {
        perror("send");
        return false;
    }

    // Step 4: Server receives client's handshake response
    bytes_received = recv(client_sock, server_msg, sizeof(server_msg), 0);
    if (bytes_received == -1) {
        perror("recv");
        return false;
    }
    server_msg[bytes_received] = '\0';
    printf("Client: %s\n", server_msg);

    printf("Handshake complete.\n");

    // Handshake complete
    return true;
}
//func to check the errors
int check(int exp, const char *msg){
    if(exp == ERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}



//[ ] Add helper functions for the main logic

int main(){
    //hash to store key value pair
    GHashTable* hash = g_hash_table_new(g_str_hash, g_str_equal);


    //thread def
    pthread_t th[THREAD_POOL_SIZE];
    pthread_mutex_init(&eventQueue_mutex, NULL);
    pthread_cond_init(&eventQueue_cond, NULL);
  
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&th[i], NULL,EventLoop, NULL) != 0) { //[x]: fix according to the helper functions.
            perror("Failed to create the thread");
        }
    }


    //socket creation
    int server_sock, client_sock,addr_size;
    SA_IN server_addr, client_addr;
    char c_addr[INET_ADDRSTRLEN];//to display the connect client's address

    check((server_sock = socket(AF_INET, SOCK_STREAM, 0)),"Failed to create socket.");

    //initializing addr struct
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    check((bind(server_sock,(struct sockaddr*) &server_addr, sizeof(server_addr))),"Failed to bind.");

    check((listen(server_sock,SERVER_BACKLOG)),"Listen failed.");

    while (true)
    {
        printf("Waiting for connections...\n");
        //waiting and accepting connections from the clients
        addr_size = sizeof(SA_IN);
        check(client_sock =
                accept(server_sock, (SA*)&client_addr,&addr_size),"Accept failed!");
        
        inet_ntop(AF_INET, &client_addr.sin_addr,c_addr,INET_ADDRSTRLEN);
        //displaying connected client info
        printf("Client connected!(IP address: %s)\n",c_addr);

        int *pclient = malloc(sizeof(int));
        *pclient = client_sock;
        //[ ]handling the connected client
        /*
        bool agree_disagree=handshake_protocol(client_sock,c_addr);

        if (agree_disagree)
        {
            continue;
        }*/


        

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