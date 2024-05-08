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
#define SERVER_STATUS // 0: accepting connections; 1: stopped connection
#define SERVER_PORT 4000
#define SERVER_IP "10.176.69.34" //"10.176.69.34"
#define SERVER_BACKLOG 6
#define THREAD_POOL_SIZE 15
#define MAX_MSG_SIZE 1024
#define HEARTBEAT_INTERVAL 10
#define MAX_SERVERS 6

// hash table for key-value store
HashTable *hash;

// struct def to store address info
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// mutex and condition vars for the threads
pthread_mutex_t eventQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t eventQueue_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t hash_locks[100]; // Array of locks for each key, assuming maximum 100 keys

// list of known server IPs
char *known_server_ips[] = {"10.176.69.35", "10.176.69.36", "10.176.69.37", "10.176.69.38", "10.176.69.39", "10.176.69.40"};

// Function prototypes
int th_key_hs(const char *key);
void *thread_task(void *arg);
int check(int exp, const char *msg);
void handle_client(void *arg);
bool is_server(const char *ip);

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

// function for handling client's request
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

    printf("\nLOG: Client communication completed.\n");
}

// Function to check if an IP belongs to any servers
bool is_server(const char *ip)
{

    int num_known_server_ips = sizeof(known_server_ips) / sizeof(known_server_ips[0]);

    for (int i = 0; i < num_known_server_ips; ++i)
    {
        if (strcmp(ip, known_server_ips[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

void *heartbeat_thread(void *arg)
{
    int server_sock;
    struct sockaddr_in server_addr;

    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // Heartbeat message
    char heartbeat_msg[] = "HEARTBEAT";

    while (true)
    {
        // Send heartbeat message to each known server
        for (int i = 0; i < MAX_SERVERS; ++i)
        {
            server_addr.sin_addr.s_addr = inet_addr(known_server_ips[i]);

            // Send heartbeat message
            if (sendto(server_sock, heartbeat_msg, sizeof(heartbeat_msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
            {
                perror("sendto failed");
                // Handle send failure
            }
        }

        // Sleep for heartbeat interval
        sleep(HEARTBEAT_INTERVAL);
    }

    close(server_sock);
    return NULL;
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
    pthread_t heartbeat_tid;

    // Init thread for sending heartbeats
    if (pthread_create(&heartbeat_tid, NULL, heartbeat_thread, NULL) != 0)
    {
        perror("Failed to create heartbeat thread");
        exit(EXIT_FAILURE);
    }

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
    int server_sock, net_sock, addr_size;
    SA_IN server_addr, net_addr;

    check((server_sock = socket(AF_INET, SOCK_STREAM, 0)), "Failed to create socket.");

    // initializing addr struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    check((bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))), "Failed to bind.");

    check((listen(server_sock, SERVER_BACKLOG)), "Listen failed.");

    char n_addr[INET_ADDRSTRLEN]; // to store connection's address

    printf("LOG: Waiting for connections...\n");

    // communication event loop
    while (true)
    {

        // waiting and accepting connections from the clients
        addr_size = sizeof(SA_IN);

        check(net_sock =
                  accept(server_sock, (SA *)&net_addr, (socklen_t *)&addr_size),
              "Accept failed!");

        inet_ntop(AF_INET, &net_addr.sin_addr, n_addr, INET_ADDRSTRLEN);

        if (is_server(n_addr))
        {
            // handle server requests.
            char heartbeat_msg[MAX_MSG_SIZE];
            ssize_t bytes_received = recv(net_sock, heartbeat_msg, sizeof(heartbeat_msg), 0);
            if (bytes_received == -1)
            {
                perror("recv");
                close(net_sock);
            }
            heartbeat_msg[bytes_received] = '\0';
            printf("Heartbeat message from server (IP: %s): %s\n", n_addr, heartbeat_msg);
        }
        else
        {
            if (rand() % 2 == 0) // close connection for 50% of incoming clients to simulate disruption
            {
                printf("LOG: Communication channel of client (IP: %s) disrupted.\n", n_addr);
                char error_message[] = "503 Service Unavailable.";
                send(net_sock, error_message, strlen(error_message), 0);
                close(net_sock); // Close the connection immediately
                continue;        // Skip handling this client
            }

            printf("\nLOG: Client with IP:%s connected!\n", n_addr);

            // enqueue client sock for the thread function to pick
            pthread_mutex_lock(&eventQueue_mutex);
            enqueue(net_sock);
            pthread_cond_signal(&eventQueue_cond);
            pthread_mutex_unlock(&eventQueue_mutex);
        }
    }

    // joining threads and destroying pthread vars
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("Failed to join the thread");
        }
    }

    pthread_join(heartbeat_tid, NULL); // Wait for the heartbeat thread to finish

    // destroying mutex and condition vars
    for (int i = 0; i < 100; i++)
    {
        pthread_mutex_destroy(&hash_locks[i]);
    }

    pthread_mutex_destroy(&eventQueue_mutex);
    pthread_cond_destroy(&eventQueue_cond);

    // Destroy the hash table
    destroyHashTable(hash);
    return 0;
}