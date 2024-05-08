// standard imports
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "hash.h"

#include "queue.h"

// for socket api and functionality
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// header to store address info
#include <netinet/in.h>

// NOTE: globals

#define ERROR (-1)
#define SERVER_STATUS 0 // 0: accepting connections(storing in mem); 1: stopped connection(writing to storage)
#define SERVER_PORT 4000
#define SERVER_IP "127.0.0.1" //"10.176.69.34"
#define SERVER_BACKLOG 6
#define THREAD_POOL_SIZE 15
#define MAX_MSG_SIZE 1024

// hash table for key-value store
HashTable *hash;

// struct def to store address info
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// mutex and condition vars for the threads
pthread_mutex_t eventQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t eventQueue_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t hash_locks[100]; // Array of locks for each key, assuming maximum 100 keys

// Function prototypes
int th_key_hs(const char *key);
void *thread_task(void *arg);
int check(int exp, const char *msg);
void handle_client(void *arg);

// NOTE: helper funcitons

// function to generate a num value for the given key using the DJB2 algorithm
int th_key_hs(const char *key)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return (int)(hash % 100); // ensure hash value is within the range of the hash_locks array size and cast to int
}

// function to execute the given thread id and close the thread once it's done
void *thread_task(void *arg)
{
    while (true)
    {
        int pclient;
        pthread_mutex_lock(&eventQueue_mutex);
        pthread_cond_wait(&eventQueue_cond, &eventQueue_mutex);
        pclient = dequeue();
        pthread_mutex_unlock(&eventQueue_mutex);

        if (pclient != ERROR)
        {
            // handling client connection
            handle_client(&pclient);
        }
    }
}

void handle_client(void *arg)
{

    int *connected_client = (int *)arg;  // cast the argument to int pointer
    int client_sock = *connected_client; // dereference the pointer to get the client socket

    // handle client request

    char client_msg[MAX_MSG_SIZE];
    ssize_t bytes_received = recv(client_sock, client_msg, sizeof(client_msg), 0);
    if (bytes_received == -1)
    {
        perror("recv");
        close(client_sock);
    }
    client_msg[bytes_received] = '\0';

    // parse client request based on delimiter
    char *operation, *key, *value;
    operation = strtok(client_msg, ".");
    key = strtok(NULL, ".");
    value = strtok(NULL, ".");

    // process client request based on operation
    char response[MAX_MSG_SIZE];
    switch (operation[0])
    {
    case 'r':
    case 'R':
        if (key == NULL)
        {
            snprintf(response, sizeof(response), "Invalid operation: key is NULL\n");
            break;
        }
        pthread_mutex_lock(&hash_locks[th_key_hs(key)]);
        char *result = hashTableGet(hash, key);
        pthread_mutex_unlock(&hash_locks[th_key_hs(key)]);

        if (result != NULL)
        {
            snprintf(response, sizeof(response), "Value for key '%s' is '%s'\n", key, result);
        }
        else
        {
            snprintf(response, sizeof(response), "Key '%s' not found\n", key);
        }
        break;
    case 'i':
    case 'I':
        if (key == NULL || value == NULL)
        {
            snprintf(response, sizeof(response), "Invalid operation: key or value is NULL\n");
            break;
        }
        pthread_mutex_lock(&hash_locks[th_key_hs(key)]);
        if (hashTableGet(hash, key) != NULL)
        {
            snprintf(response, sizeof(response), "Key '%s' already exists, use 'u' to update\n", key);
        }
        else
        {
            hashTablePut(hash, key, value);
            snprintf(response, sizeof(response), "Value '%s' stored for key '%s'\n", value, key);
        }
        pthread_mutex_unlock(&hash_locks[th_key_hs(key)]);
        break;
    case 'u':
    case 'U':
        if (key == NULL || value == NULL)
        {
            snprintf(response, sizeof(response), "Invalid operation: key or value is NULL\n");
            break;
        }
        pthread_mutex_lock(&hash_locks[th_key_hs(key)]);
        if (hashTableGet(hash, key) != NULL)
        {
            hashTablePut(hash, key, value);
            snprintf(response, sizeof(response), "Value '%s' updated for key '%s'\n", value, key);
        }
        else
        {
            snprintf(response, sizeof(response), "Key '%s' not found\n", key);
        }
        pthread_mutex_unlock(&hash_locks[th_key_hs(key)]);
        break;
    default:
        snprintf(response, sizeof(response), "Invalid operation\n");
        break;
    }

    ssize_t bytes_sent = send(client_sock, response, strlen(response), 0);
    if (bytes_sent == -1)
    {
        perror("send");
    }
}

// function to check the errors
int check(int exp, const char *msg)
{
    if (exp == ERROR)
    {
        perror(msg);
        exit(1);
    }
    return exp;
}

int main()
{
    // thread def
    pthread_t th[THREAD_POOL_SIZE];
    pthread_mutex_init(&eventQueue_mutex, NULL);
    pthread_cond_init(&eventQueue_cond, NULL);

    // initialize locks for hash table keys
    for (int i = 0; i < 10; i++)
    {
        pthread_mutex_init(&hash_locks[i], NULL);
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (pthread_create(&th[i], NULL, thread_task, NULL) != 0)
        {
            perror("Failed to create the thread");
        }
    }

    // create the hash table
    hash = createHashTable();

    // populating the hash table with initial key-value pairs
    hashTablePut(hash, "car", "Audi");
    hashTablePut(hash, "movie", "Interstellar");
    hashTablePut(hash, "bike", "Ducati");
    // socket creation
    int server_sock, client_sock, addr_size;
    SA_IN server_addr, client_addr;

    check((server_sock = socket(AF_INET, SOCK_STREAM, 0)), "Failed to create socket.");

    // initializing addr struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    check((bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))), "Failed to bind.");

    check((listen(server_sock, SERVER_BACKLOG)), "Listen failed.");
    
    char c_addr[INET_ADDRSTRLEN];

    // event loop
    while (true)
    {

        printf("Waiting for connections...\n");
        // waiting and accepting connections from the clients
        addr_size = sizeof(SA_IN);
        check(client_sock =
                  accept(server_sock, (SA *)&client_addr, (socklen_t *)&addr_size),
              "Accept failed!");

        inet_ntop(AF_INET, &client_addr.sin_addr, c_addr, INET_ADDRSTRLEN);
        if (rand() % 2 == 0) // close connection for 50% of incoming clients during disruption
        {
            printf("Communication channel of client (IP: %s) disrupted.\n", c_addr);
            char error_message[] = "503 Service Unavailable.";
            send(client_sock, error_message, strlen(error_message), 0);
            close(client_sock); // Close the connection immediately
            continue;           // Skip handling this client
        }

        printf("\nconnected!\n");

        pthread_mutex_lock(&eventQueue_mutex);
        enqueue(client_sock);
        pthread_cond_signal(&eventQueue_cond);
        pthread_mutex_unlock(&eventQueue_mutex);
    }

    // joining threads and destroying pthread vars
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("Failed to join the thread");
        }
    }

    // destroying mutex and condition vars
    for (int i = 0; i < 100; i++)
    {
        pthread_mutex_destroy(&hash_locks[i]);
    }

    pthread_mutex_destroy(&eventQueue_mutex);
    pthread_cond_destroy(&eventQueue_cond);

    return 0;
}