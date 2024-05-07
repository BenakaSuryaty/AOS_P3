#ifndef QUEUE_H_
#define QUEUE_H_

// Define node structure
typedef struct node
{
    struct node* next;
    int client_sock;
} node_t;

// Function prototypes
void enqueue(int client_sock);
int dequeue();

#endif /* QUEUE_H_ */