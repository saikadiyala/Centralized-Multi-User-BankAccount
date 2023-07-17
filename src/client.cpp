#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>
#include<iostream>
#include<chrono>
#include<ctime>
  
int arr_time[2];
double sum;
using namespace std;
FILE *fp;
void * cientThread(void *arg)
{
  //getting the timestamp
  double time_st = *((double*)arg);
    
  char buffer[4096];
  int clientSocket;
  char msg[1000];
  struct sockaddr_in serverAddr;
  socklen_t addr_size;
  
  //creating a temporarty var to get the time from the line of req
  
  char temp[100];
  

  // Create the socket. 
  clientSocket = socket(PF_INET, SOCK_STREAM, 0);

  //Configure settings of the server address
 // Address family is Internet 
  serverAddr.sin_family = AF_INET;

  //Set port number, using htons function 
  serverAddr.sin_port = htons(8989);

 //Set IP address to localhost
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    //Connect the socket to the server using the address
    addr_size = sizeof serverAddr;
    connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
    
	if(fgets(msg,1000,fp) != NULL){
	
	strcpy(temp,msg);
	
	//taking timestamp as factor to make the thread sleep before sending the data to the server
	
	sleep((atoi(strtok(temp, " ")) - arr_time[0])*time_st);
	
	//starting timer
	std::chrono::steady_clock::time_point _start(std::chrono::steady_clock::now());			
	if( send(clientSocket,msg,strlen(msg),0)<0) {
		cout<<"Send failed"<<endl;
	}
	else{
		cout<<"Sending data was a success from the client side for-"<<msg<<endl;
	}
	
	//bzero(buffer,4096);
	if(recv(clientSocket,buffer,1024,0)<0) {
	
		cout<<"Received failed"<<endl;
	}
	else{
	cout<<"Ackgmnt from server is---"<<buffer<<endl;
	
	}
	
	//finding the difference between the start and end point to get the time taken
	//for each tx request
	std::chrono::steady_clock::time_point _end(std::chrono::steady_clock::now());
    
    
    sum +=std::chrono::duration_cast<std::chrono::duration<double>>( _end - _start).count() ;
    }
    cout<<"total time taken for all the txs that are executed till now---"<< sum<<endl;
    cout<<"-----"<<endl;
    close(clientSocket);
    pthread_exit(NULL);
}
int main(int arg,char* args[]){
  int i = 0;
  int count = 0;
  char buffer[1000];
  string path;
  double* timestamp;

  
  if(arg == 1 ) {
  	
  	cout<<"OOPS..no args have been passed"<<endl;
  	return 0;
  }
  else if(arg == 2 ) {
  
  	cout<<"Only file argument has been passed"<<endl;
  	path = args[1];
  	*timestamp = 0.0;
  }
  
  
  else if(arg == 3 ) {
  
  cout<<"Looks like a file arg and timestamp arg have been passed"<<endl;
  path = args[1];
  *timestamp = atof(args[2]);  
  }
  
  string line;
  fp = fopen(path.c_str(),"r");
  int line_no = 0;
  
  //cout<<"Timestamp"<<timestamp;
  
  
  cout<<"Given path of the file"<<path.c_str()<<endl<<"--------"<<endl;
  
  //get the number of tx req present in the file to create
  //respective number of threads
if(fp!=NULL) {
	while(fgets(buffer,1000,fp) != NULL){
		if(line_no == 0) {
			
			arr_time[0] = atoi(strtok(buffer," "));
		}
		//cout<<"The "<<count<<" line is "<<buffer<<endl;
		count++;
		bzero(buffer,1000);
		//cout<<"The initial time is:"<<arr_time[0]<<endl;
		line_no++;
	}

}

else{
cout<<"Error in opening the file,please check the path"<<endl;
return 1;
}

//resetting the file pointer to the start
rewind(fp);
	
/* Thread creation and using joint were reffered from Dzone.com/articles/parallel-tcpip-socket-server-with-multi-threading */


 //creating threads for the tx reqs
  pthread_t tid[count+1];
  while(i< count)
  {
    if( pthread_create(&tid[i], NULL, cientThread, timestamp) != 0 )
           printf("Failed to create thread\n");
    i++;
  }
  
  //to give sometime to complete the execution of all the threads
  cout<<"---------------"<<endl;
  sleep(4);
  i = 0;
  
  //thread join
  while(i< count)
  {
     pthread_join(tid[i++],NULL);
  }
  
  //close the file pointer
  fclose(fp);
  return 0;
}
