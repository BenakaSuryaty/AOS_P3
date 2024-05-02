// Standard imports
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include<string.h>
#include <stdbool.h>
#include <pthread.h>


// For socket api and functionality
#include<sys/types.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#define SOCKETERROR (-1)

// Header and global to store address info in a struct
#include<netinet/in.h>
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// struct to store 
    struct AcceptedSocket{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
    };


// Func to check the socket connection errors
int check_soc(int exp, const char *msg){
    if(exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}

void main(){
    
}