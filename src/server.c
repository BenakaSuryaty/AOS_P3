// standard imports
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <glib.h>

#include "queue.h"

// for socket api and functionality
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// header to store address info
#include <netinet/in.h>

// NOTE: globals

#define ERROR (-1)
#define SERVER_STATUS 0       // 0: accepting connections(storing in mem); 1: stopped connection(writing to storage)
#define SERVER_PORT 4000      //[ ]: change to dc machine's available port
#define SERVER_IP "127.0.0.1" //[ ]: change to dc machine's IP
#define SERVER_BACKLOG 6
#define THREAD_POOL_SIZE 15
#define MAX_MSG_SIZE 1024

// hash table for key-value store
GHashTable *hash;

// struct def to store address info
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// mutex and condition vars for the threads
pthread_mutex_t eventQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t eventQueue_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t hash_locks[100]; // Array of locks for each key, assuming maximum 100 keys

// Function prototypes
int hash_func(const char *key);
void *thread_task(void *arg);
int check(int exp, const char *msg);
void handle_client(void *arg);

// NOTE: helper funcitons

// function to generate a hash value for the given key using the DJB2 algorithm
int hash_func(const char *key)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return (int)(hash % 100); // Ensure hash value is within the range of the hash_locks array size and cast to int
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

            handle_client(&pclient);
        }
    }
}

void handle_client(void *arg)
{

    int *connected_client = (int *)arg;  // Cast the argument to int pointer
    int client_sock = *connected_client; // Dereference the pointer to get the client socket

    // Handle client request

    char client_msg[MAX_MSG_SIZE];
    ssize_t bytes_received = recv(client_sock, client_msg, sizeof(client_msg), 0);
    if (bytes_received == -1)
    {
        perror("recv");
        close(client_sock);
    }
    client_msg[bytes_received] = '\0';

    // Parse client request based on delimiter
    char *operation, *key, *value;
    operation = strtok(client_msg, ".");
    key = strtok(NULL, ".");
    value = strtok(NULL, ".");

    // Process client request based on operation
    char response[MAX_MSG_SIZE];
    switch (operation[0])
    {
    case 'r':
    case 'R':

        pthread_mutex_lock(&hash_locks[hash_func(key)]);
        char *result = g_hash_table_lookup(hash, key);
        pthread_mutex_unlock(&hash_locks[hash_func(key)]);

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

        pthread_mutex_lock(&hash_locks[hash_func(key)]);
        g_hash_table_replace(hash, g_strdup(key), g_strdup(value));
        pthread_mutex_unlock(&hash_locks[hash_func(key)]);

        snprintf(response, sizeof(response), "Value '%s' stored for key '%s'\n", value, key);
        break;
    case 'u':
    case 'U':

        pthread_mutex_lock(&hash_locks[hash_func(key)]);
        g_hash_table_replace(hash, g_strdup(key), g_strdup(value));
        pthread_mutex_unlock(&hash_locks[hash_func(key)]);

        snprintf(response, sizeof(response), "Value '%s' updated for key '%s'\n", value, key);
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

    // Initialize locks for hash table keys
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

    // hash table definition
    hash = g_hash_table_new(g_str_hash, g_str_equal);

    // populating the hash table with initial key-value pairs
    g_hash_table_insert(hash, "car", "Audi");
    g_hash_table_insert(hash, "movie", "Interstellar");
    g_hash_table_insert(hash, "bike", "Ducati");

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

    // event loop
    while (true)
    {

        printf("Waiting for connections...\n");
        // waiting and accepting connections from the clients
        addr_size = sizeof(SA_IN);
        check(client_sock =
                  accept(server_sock, (SA *)&client_addr, (socklen_t *)&addr_size),
              "Accept failed!");
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

    // Freeing hash table memory
    g_hash_table_destroy(hash);
    return 0;
}