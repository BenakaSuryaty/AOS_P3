#include "queue.h"
#include <stdlib.h>

node_t* head = NULL;
node_t* tail = NULL;

void enqueue(int client_sock) {
    // Allocate memory for a new node
    node_t *newnode = malloc(sizeof(node_t));
    if (newnode == NULL) {
        // Memory allocation failed
        return;
    }

    // Initialize the new node
    newnode->client_sock = client_sock;
    newnode->next = NULL;

    // Update the tail pointer
    if (tail == NULL) {
        head = newnode;
    } else {
        tail->next = newnode;
    }
    tail = newnode;
}

int dequeue() {
    if (head == NULL) {
        // Queue is empty
        return -1; // Or any appropriate error code
    } else {
        int result = head->client_sock;
        node_t *temp = head;
        head = head->next;
        if (head == NULL) {
            tail = NULL;
        }

        free(temp);
        return result;
    }
}
