#ifndef QUEUE_H_
#define QUEUE_H_

struct node
{
    struct node* next;
    int *client_sock;
};

typedef struct node node_t;

// Function prototypes
void enqueue(int *client_sock);
int* dequeue();

#endif