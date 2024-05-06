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

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

//creating another threadpool to handle all the different threads
class ThreadPool
{
	public:
		using Task = function<void()>;

		template<class T>
		auto enqueue(T task)->future<decltype(task())>
		{

			auto wrapper = make_shared<packaged_task<decltype(task()) ()> >(move(task));
		
			{
				unique_lock<mutex> lock{EventMutex};
				tasks.emplace([=] {
						(*wrapper)();
						});
			}
			
			mEventVar.notify_one();

			return wrapper->get_future();

		
		
		}


		explicit ThreadPool(size_t numThreads)
		{
			start(numThreads);

		}

		~ThreadPool()
		{

			stop();

		}



	private:
		queue<Task> tasks;

		condition_variable mEventVar;

		mutex EventMutex;

		bool stopping = false;	
		vector <thread> mthreads;
	void start(size_t numThreads)
	{
		for(auto i =0; i < numThreads; i++)
		{	
			mthreads.emplace_back([=] {

				while(true)
				{
					Task task;

					{
						unique_lock<mutex> lock{EventMutex};

						mEventVar.wait(lock, [=] {return stopping || !tasks.empty();});

						if(stopping && tasks.empty() )
							break;

						task =move(tasks.front());
						tasks.pop();
							
					}

					task();		
				}
				
					
					
			});


		}

	}

	void stop() noexcept
	{
		{
			unique_lock<mutex> lock{EventMutex};
			stopping = true;
		}
		mEventVar.notify_all();
		
		for(auto &thread:mthreads)
		{
			thread.join();
		}
	}

};


int hashFunction(const std::string& obj);

string createFile();

void readFile(string fileName);

int clientRead(int hashNum, string fileName);

void converStringToChar(char* timeChar, string timeStr);

string getkeyVal(const std::string& obj);

int main(){

	//variable for menu selection
	int selection;
	cout << "Hello" << endl;

	
	//creating the threadpool
	ThreadPool pool{10};
	
	pool.enqueue([] {
		cout << "Hello from thread" << endl;		
	});

	cout << hashFunction("Hello");
	
	while(true){


		//outputting menu
		cout << "\n1. Create and edit a file" << endl;
		cout << "2. Read a file" << endl;
		cout << "3. Edit a file" << endl;
		cout << "4. Exit program" << endl;

		cin >> selection;

		//code for menu
		if(selection == 1)
		{
			cout << hashFunction(createFile()) << endl;

		}
		else if (selection ==2)
		{
			//getting fileName and value for hash
			int hashNum;
			string fileName;
			
			cout<< "Enter file you'd like to read: ";
			cin >> fileName;
			readFile(fileName);

			//getting the first hashed value
			hashNum = hashFunction(fileName);

			//attempting to connect with the clients
			if(clientRead(hashNum %7, fileName) < 0)
			{
				cout << "first server down unreachable, connecting to next" << endl;
				if(clientRead((hashNum +2) % 7, fileName) < 0)
				{
					cout << "second server unreachable, connecting to next" << endl;
					
					if(clientRead((hashNum +4) % 7, fileName) < 0)
					{
						cout << "None of the servers are reachable, aborting write" << endl;
					}
				}
			}
			

		}
		else if (selection ==3)
		{

		}
		else if ( selection ==4)
		{
			break;
		}
		else 
		{
			cout << "Error: Invalid selection" << endl;
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

string createFile(){
	
	string fileName;
	string content;

	
	cout << "Enter file Name: " << endl;
	cin >> fileName;

	//creating file
	ofstream MyFile(fileName);
	
	cout << "Enter file content: ";
	cin >> content;

	MyFile << content;


	//closing the file
	MyFile.close();

	return fileName;

}


void readFile(string fileName){
// Create a text string, which is used to output the text file

       	string myText;


       	// Read from the text file

       	ifstream MyReadFile(fileName);


       	// Use a while loop together with the getline() function to read the file line by line

       	while (getline (MyReadFile, myText)) {

	     	// Output the text from the file

	   	cout << myText;
     }

     // Close the file
     MyReadFile.close();

}



int clientRead(int hashVal, string fileName)
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
	

     //waiting for ack
      n = read(sockfd,buffer,255);



      //creating the request
      char request_char[100];

      //getting the timestamp
      milliseconds ms = duration_cast< milliseconds >(
		    system_clock::now().time_since_epoch()
		);

      string timestampString  = to_string(ms.count());

      //getting unique keyvalue from hashing fileName
     string key = getkeyVal(fileName);

    string request = timestampString + ":get:" + key;

	converStringToChar(request_char, request); 

  	n = write(sockfd, request_char,100);


        

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


