#include "queue.h"
#include<stdlib.h>

node_t* head = NULL;
node_t* tail = NULL;

void enqueue(int *client_sock){
    
    if (client_sock == NULL) {
        // Invalid client_sock pointer
        return;
    }

    node_t *newnode = malloc(sizeof(node_t));
    
    if (newnode == NULL) {
        // Memory allocation failed
        return;
    }


    /*
    newnode->client_sock = client_sock;
    newnode->next = NULL;
    */
    // Allocate memory for client_sock and copy the value
    newnode->client_sock = malloc(sizeof(int));
    if (newnode->client_sock == NULL) {
        // Memory allocation failed
        free(newnode);
        return;
    }
    
    *(newnode->client_sock) = *client_sock;
    newnode->next = NULL;

    if (tail==NULL){
        head = newnode;
    }   else
    {
        tail->next = newnode;
    }
    tail = newnode;
}

int* dequeue(){
    if(head==NULL){
        // Queue is empty
        return NULL;
    }   else
    {
        int *result = head->client_sock;
        node_t *temp = head;
        head = head->next;
        if (head==NULL){
            tail=NULL;
        }
        
        free(temp->client_sock);
        free(temp);
        
        return result;
    }
}