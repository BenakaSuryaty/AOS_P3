#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <thread>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include <pthread.h>
#include <queue>
#include <vector>
#include <future>
#include <condition_variable>
#include <sys/wait.h>
#include <chrono>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sys/time.h>

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;



#define MAX_HANDSHAKE_MSG_SIZE 1024

//functions used
int hashFunction(const std::string& obj);

string createFile();

void readFile(string fileName);

int clientRead(int hashNum, string fileName);

void converStringToChar(char* timeChar, string timeStr);

string getkeyVal(const std::string& obj);

int clientWrite(int hashVal, string fileName);

int clientInsert(int hashVal, string keyName);

bool handshake_protocol(int server_sock); 

int main(){

	//variable for menu selection
	int selection;

	
	
	while(true){


		//outputting menu
		cout << "\n1. Insert new key" << endl;
		cout << "2. Read a Key" << endl;
		cout << "3. Update Key" << endl;
		cout << "4. Exit program" << endl;

		cin >> selection;

		switch (selection)
		{
			case 1: 
				{
				//getting fileName and value for hash
		
				int hashNum;
		
				string keyName;
		
				cout<< "Enter the key name you'd like to insert: ";
				cin >> keyName;

				//getting the first hashed value
				hashNum = hashFunction(keyName);

				//attempting to connect with the clients
				if(clientInsert(hashNum %7, keyName) < 0)
				{
			
					cout << "first server down unreachable, connecting to next" << endl;
				
					if(clientInsert((hashNum +2) % 7, keyName) < 0)
					{

					
						cout << "second server unreachable, connecting to next" << endl;
					
					
						if(clientInsert((hashNum +4) % 7, keyName) < 0)
						{
							cout << "None of the servers are reachable, aborting write" << endl;
						}
				
			
					}
				}
				
				break;
				}
			case 2:
			{
					//getting fileName and value for hash
				int hashNum;
				string keyName; 
			
				cout<< "Enter key you'd like to read: ";
				cin >> keyName;

				//getting the first hashed value
				hashNum = hashFunction(keyName);

				//attempting to connect with the clients
				if(clientRead(hashNum %7, keyName) < 0)
				{
					cout << "first server down unreachable, connecting to next" << endl;
					if(clientRead((hashNum +2) % 7, keyName) < 0)
					{
						cout << "second server unreachable, connecting to next" << endl;
					
						if(clientRead((hashNum +4) % 7, keyName) < 0)
						{
							cout << "None of the servers are reachable, aborting write" << endl;
						}
					}
				}
			
				break;
			}
			case 3:
			{
				//getting fileName and value for hash
				int hashNum;
				string keyName;
			
				cout<< "Enter key you'd like to update: ";
				cin >> keyName;

				//getting the first hashed value
				hashNum = hashFunction(keyName);

				//attempting to connect with the clients
				if(clientRead(hashNum %7, keyName) < 0)
				{
					cout << "first server down unreachable, connecting to next" << endl;
					if(clientRead((hashNum +2) % 7, keyName) < 0)
					{
						cout << "second server unreachable, connecting to next" << endl;
					
						if(clientRead((hashNum +4) % 7, keyName) < 0)
						{
							cout << "None of the servers are reachable, aborting write" << endl;
						}
					}
				}
			
				break;

			}
			case 4:
			{

				cout << "Exitting program" << endl;
				return 0;
			}
			default:
			{
				cout << "Invalid Entry, try again" << endl;
				break;

			}
			



		}
		

	}



}



int hashFunction(const std::string& obj) {
	    // Calculate the hash value based on the object
     int sum = 0;

     for (char c : obj) {

	     sum += static_cast<int>(c);

     }

     // Map the hash value to the range 0 to 6

     return sum ;


}



int clientRead(int hashVal, string keyName)
{
      int portArr[8] = {2312, 2313, 2314, 2315, 2316, 2317,2318, 4000};
	string machineName[8] = {"dc01.utdallas.edu", "dc02.utdallas.edu", "dc03.utdallas.edu", "dc04.utdallas.edu", "dc05.utdallas.edu", "dc06.utdallas.edu", "dc07.utdallas.edu", "dc03.utdallas.edu"};
     int sockfd, portno, n;
     struct sockaddr_in serv_addr;
     struct hostent *server;
     char buffer[256];

     //change the port arre to hasNum
     portno = (portArr[hashVal]);
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
	    printf("ERROR opening socket");
	    return -1;
     }

     server = gethostbyname(machineName[hashVal].c_str());

     if (server == NULL) {
	     fprintf(stderr,"ERROR, no such host\n");
	     return -1;
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;

     bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

     serv_addr.sin_port = htons(portno);
     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
     {
	     printf("ERROR connecting");
	     return -1;
     }


     //creating the variables needed to send request
	size_t bytes_received, bytes_sent;

    	char server_msg[MAX_HANDSHAKE_MSG_SIZE];
        char client_request[MAX_HANDSHAKE_MSG_SIZE] = "r.car.";
	string value;
	bzero(buffer, 255);
    
   
      //communicating to server
      string request = "read." + keyName + ".NULL";

      converStringToChar(client_request, request); 
    	bytes_sent = send(sockfd, client_request, strlen(client_request), 0);
	if (bytes_sent == -1) {
		perror("send");
		return false;
	}


	
//	Receive acknowledgment from server
	bytes_received = recv(sockfd, server_msg, sizeof(server_msg), 0);
	
	if (bytes_received == -1) {
		perror("recv");
		return false;
	}

	server_msg[bytes_received] = '\0';
	printf("Server: %s\n", server_msg);


	return 1;

}


//function to send update request
int clientWrite(int hashVal, string keyName)
{
	//arr storying all server names and portnumbers
    int portArr[8] = {2312, 2313, 2314, 2315, 2316, 2317,2318, 4000};
	string machineName[8] = {"dc01.utdallas.edu", "dc02.utdallas.edu", "dc03.utdallas.edu", "dc04.utdallas.edu", "dc05.utdallas.edu", "dc06.utdallas.edu", "dc07.utdallas.edu", "dc03.utdallas.edu"};

//	storing the variables needed to create client
     int sockfd, portno, n;
     struct sockaddr_in serv_addr;
     struct hostent *server;
     char buffer[256];

    //creating client
     portno = (portArr[hashVal]);
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
	    printf("ERROR opening socket");
	    return -1;
     }

     server = gethostbyname(machineName[hashVal].c_str());

     if (server == NULL) {
	     fprintf(stderr,"ERROR, no such host\n");
	     return -1;
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

     serv_addr.sin_port = htons(portno);
     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
     {
	     printf("ERROR connecting");
	     return -1;
     }


	//creating the variables needed to send request
	size_t bytes_received, bytes_sent;

    	char server_msg[MAX_HANDSHAKE_MSG_SIZE];
        char client_request[MAX_HANDSHAKE_MSG_SIZE] = "r.car.";
	string value;
	bzero(buffer, 255);
    
      	//getting the value to insert
	cout << "Enter value to be updated: ";
  	cin >> value;
   
      //communicating to server
      string request = "update." + keyName + "." + value;

      
      converStringToChar(client_request, request); 

    	bytes_sent = send(sockfd, client_request, strlen(client_request), 0);
	if (bytes_sent == -1) {
		perror("send");
		return false;
	}


	
//	Receive acknowledgment from server
	bytes_received = recv(sockfd, server_msg, sizeof(server_msg), 0);
	
	if (bytes_received == -1) {
		perror("recv");
		return false;
	}

	server_msg[bytes_received] = '\0';
	printf("Server: %s\n", server_msg);

	return 1;

}


//function to send update request
int clientInsert(int hashVal, string keyName)
{


	//arr storying all server names and portnumbers
    int portArr[8] = {2312, 2313, 2314, 2315, 2316, 2317,2318, 4000};
	string machineName[8] = {"dc01.utdallas.edu", "dc02.utdallas.edu", "dc03.utdallas.edu", "dc04.utdallas.edu", "dc05.utdallas.edu", "dc06.utdallas.edu", "dc07.utdallas.edu", "dc03.utdallas.edu"};

//	storing the variables needed to create client
     int sockfd, portno, n;    
     struct sockaddr_in serv_addr;    
     struct hostent *server;
     char buffer[256];

    //creating client
     portno = (portArr[hashVal]);
     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0) {

	    printf("ERROR opening socket\n");
	    return -1;
     }

     server = gethostbyname(machineName[hashVal].c_str());

     if (server == NULL) {
	     fprintf(stderr,"ERROR, no such host\n");
	     return -1;
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

     serv_addr.sin_port = htons(portno);
     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
     {
	     printf("ERROR connecting \n");
	     return -1;
     }
	
 

	//creating the variables needed to send request
	size_t bytes_received, bytes_sent;

    	char server_msg[MAX_HANDSHAKE_MSG_SIZE];
        char client_request[MAX_HANDSHAKE_MSG_SIZE] = "r.car.";
	string value;
	bzero(buffer, 255);
    
      	//getting the value to insert
	cout << "Enter value to be inserted: ";
  	cin >> value;
   
      //communicating to server
      string request = "insert." + keyName + "." + value;

      
      converStringToChar(client_request, request); 

    	bytes_sent = send(sockfd, client_request, strlen(client_request), 0);

	if (bytes_sent == -1) {

		perror("send");

		return false;

	}


	
//	Receive acknowledgment from server

	bytes_received = recv(sockfd, server_msg, sizeof(server_msg), 0);

	
	if (bytes_received == -1) {


		perror("recv");

		return false;

	}

	server_msg[bytes_received] = '\0';
	printf("Server: %s\n", server_msg);
	
	return 1;


}


void converStringToChar(char* timeChar, string timeStr){
	    
	for(int i=0; i < timeStr.length(); ++i){
		        timeChar[i] = timeStr[i];
			  }
}



string getkeyVal(const std::string& obj) {
	    // Calculate the hash value based on the object
     int sum = 0;

     for (char c : obj) {

	     sum += static_cast<int>(c);

     }

     return to_string(sum);
 


}




bool handshake_protocol(int server_sock) {
	char server_msg[MAX_HANDSHAKE_MSG_SIZE];
	char client_msg[MAX_HANDSHAKE_MSG_SIZE];
	ssize_t bytes_received, bytes_sent;


	
	printf("Handshake initiated.\n");


	
	const char* client_hello = "Requesting connection.\n";
	bytes_sent = send(server_sock, client_hello, strlen(client_hello), 0);
	if (bytes_sent == -1) {
		perror("send");
		return false;
	}


//	 Receive acknowledgment from server
	 bytes_received = recv(server_sock, server_msg, sizeof(server_msg), 0);
	 if (bytes_received == -1) {

                perror("recv");
		return false;
	 }
	 server_msg[bytes_received] = '\0';
	 printf("Server: %s\n", server_msg);

	 const char* client_response = "Connection established.\n";
	 bytes_sent = send(server_sock, client_response, strlen(client_response), 0);
	 if (bytes_sent == -1) {

		 perror("send");
		 return false;
	 }

	 printf("Handshake complete.\n");
	 printf("Connected to server!\n");
	 // Handshake complete
	 return true;

}



