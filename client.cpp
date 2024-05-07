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



      int portArr[7] = {2312, 2313, 2314, 2315, 2316, 2317,2318};
	string machineName[7] = {"dc01.utdallas.edu", "dc02.utdallas.edu", "dc03.utdallas.edu", "dc04.utdallas.edu", "dc05.utdallas.edu", "dc06.utdallas.edu", "dc07.utdallas.edu"};

     int sockfd, portno, n;
     
     struct sockaddr_in serv_addr;
     
     struct hostent *server;



     char buffer[256];

    
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

     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
     {
	     printf("ERROR connecting");
	     return -1;
     }
	

   /*  //handshake protocol
     //
    fd_set socketfd;
    FD_ZERO(&socketfd);
    FD_SET(sockfd, &socketfd);

    //setting timeoutvalue
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
	
      n = write(sockfd, "Sending ack", 20);


      //waiting for ack message
      int ret = select(sockfd+1, &socketfd, NULL, NULL, &timeout);


      if(ret > 0){

     	 n = read(sockfd,buffer,255);
      }
      else
      {
	cout << "No message from server, abort" << endl;
	return -1;
      }
*/
      if(!handshake_protocol(sockfd))
      {
	cout << "Handshake not completed, aborting" << endl;
      }

      //creating the request
      char request_char[100];

      //getting the timestamp
      milliseconds ms = duration_cast< milliseconds >(
		    system_clock::now().time_since_epoch()
		);

      string timestampString  = to_string(ms.count());

      //getting unique keyvalue from hashing fileName
     string key = getkeyVal(keyName);


     //update or read
    string request = timestampString + "." +  keyName + "."  + "read";

	converStringToChar(request_char, request); 

  	n = write(sockfd, request_char,100);

	n = read(sockfd, buffer, 255);

	printf("Here is the Key value: %s\n", buffer);	

	return 0;
	
        

}


//function to send update request
int clientWrite(int hashVal, string keyName)
{


	//arr storying all server names and portnumbers
      int portArr[7] = {2312, 2313, 2314, 2315, 2316, 2317,2318};
	string machineName[7] = {"dc01.utdallas.edu", "dc02.utdallas.edu", "dc03.utdallas.edu", "dc04.utdallas.edu", "dc05.utdallas.edu", "dc06.utdallas.edu", "dc07.utdallas.edu"};


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

     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
     {
	     printf("ERROR connecting");
	     return -1;
     }
	

     //handshake protocol
     //
  /*  fd_set socketfd;
    FD_ZERO(&socketfd);
    FD_SET(sockfd, &socketfd);

    //setting timeoutvalue
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
	
      n = write(sockfd, "Sending ack", 20);


      //waiting for ack message
      int ret = select(sockfd+1, &socketfd, NULL, NULL, &timeout);


      if(ret > 0){

     	 n = read(sockfd,buffer,255);
      }
      else
      {
	cout << "No message from server, abort" << endl;
	return -1;
      }*/

	if(!handshake_protocol(sockfd))
	{
		cout << "Handshake not completed, returning back to main\n";
		return -1;	
	}

      //creating the request
      char request_char[100];

      //getting the timestamp
      milliseconds ms = duration_cast< milliseconds >(
		    system_clock::now().time_since_epoch()
		);

      string timestampString  = to_string(ms.count());


      //communicating to server
    string request = timestampString + "." + keyName + ".update";

	converStringToChar(request_char, request); 

  	n = write(sockfd, request_char,100);

	//getting reques to update value
	n = read(sockfd, buffer, 255);

	printf( buffer);

	//prompting user to enter updated value and creating variables for that process.
	string keyValue;
	bzero(buffer, 256);
	cin >> keyValue;	
	
	converStringToChar(buffer, keyValue);

	//sebdubg the updated valuei and getting confirmation
	write(sockfd, buffer, 255);

	bzero(buffer, 256);

	read(sockfd, buffer, 255);

	printf(buffer);

	return 1;
	

        

}


//function to send update request
int clientInsert(int hashVal, string keyName)
{


	//arr storying all server names and portnumbers
      int portArr[7] = {2312, 2313, 2314, 2315, 2316, 2317,2318};
	string machineName[7] = {"dc01.utdallas.edu", "dc02.utdallas.edu", "dc03.utdallas.edu", "dc04.utdallas.edu", "dc05.utdallas.edu", "dc06.utdallas.edu", "dc07.utdallas.edu"};


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

     if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
     {
	     printf("ERROR connecting");
	     return -1;
     }
	

     //handshake protocol
     //
   /* fd_set socketfd;
    FD_ZERO(&socketfd);
    FD_SET(sockfd, &socketfd);

    //setting timeoutvalue
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
	
      n = write(sockfd, "Sending ack", 20);


      //waiting for ack message
      int ret = select(sockfd+1, &socketfd, NULL, NULL, &timeout);


      if(ret > 0){

     	 n = read(sockfd,buffer,255);
      }
      else
      {
	cout << "No message from server, abort" << endl;
	return -1;
      }
*/
	if(!handshake_protocol(sockfd))
	{
		cout << "Handshake not completed, returning back to main\n";
		return -1;	
	}

      //creating the request
      char request_char[100];

      //getting the timestamp
      milliseconds ms = duration_cast< milliseconds >(
		    system_clock::now().time_since_epoch()
		);

      string timestampString  = to_string(ms.count());


      //communicating to server
    string request = timestampString + "." + keyName + ".insert";

	converStringToChar(request_char, request); 

  	n = write(sockfd, request_char,100);
	printf( buffer);

	//getting value to be sent.
	string keyValue;
	bzero(buffer, 256);
	cin >> keyValue;	
	
	converStringToChar(buffer, keyValue);

	//sebdubg the inserted valuei and getting confirmation
	write(sockfd, buffer, 255);

	bzero(buffer, 256);

	read(sockfd, buffer, 255);

	printf(buffer);
	

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

	// Step 1: Client receives the server's handshake message

	     bytes_received = recv(server_sock, server_msg, sizeof(server_msg), 0);

	         if (bytes_received == -1) {
                 
			 perror("recv");
                         return false;
	   	 }

                        
		 server_msg[bytes_received] = '\0';
        
		 printf("Server: %s\n", server_msg);

		 // Step 2: Client sends its handshake message
		 const char* client_hello = "Hello! Here is my handshake message.\n";
		 bytes_sent = send(server_sock, client_hello, strlen(client_hello), 0);
		 if (bytes_sent == -1) {

			 perror("send");

			 return false;

		 }

		 // Step 3: Client receives the server's response
		 bytes_received = recv(server_sock, server_msg, sizeof(server_msg), 0);
		 if (bytes_received == -1) {

			 perror("recv");

			 return false;
		 }



		 server_msg[bytes_received] = '\0';



		 printf("Server: %s\n", server_msg);





		 // Step 4: Client sends its confirmation

		 const char* client_response = "Received server response. Handshake complete.\n";

		 bytes_sent = send(server_sock, client_response, strlen(client_response), 0);



		 if (bytes_sent == -1) {

			 perror("send");

			 return false;

		 }



		 printf("Handshake complete.\n");

		 // Handshake complete

		 return true;

}




