//g++ -g finalserver.cpp myqu.cpp -o finalserver -pthread

#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdbool.h>
#include<limits.h>
#include<pthread.h>
#include<iostream>
#include "../header/mque.h"
#include"unistd.h"
#include<string.h>
#include<mutex>
#include<fstream>
#include <bits/stdc++.h>



//to store the bank details of a customer
struct bankrecords {

	int account_no;
	char name[100];
	int balance;
}data[500];

using namespace std;

//to store the file path
string path;

#define SERVERPORT 8989
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 10
#define THREAD_POOL 20

//declaring the mutex locks for CS and the conditional variables
pthread_t thread_box[THREAD_POOL];
pthread_mutex_t mutex_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

std::mutex got;


typedef struct sockaddr_in  SA_IN;
typedef struct sockaddr SA;

void handle_connection(int  *p_client_socket, char* path);
int error_check(int exp, const char *msg);
void *thread_func(void *arg);


//load the data from file into the data records
int loadthedata(string path) {

	FILE *fp;
	
	fp = fopen(path.c_str(),"r");
	
	if(fp ==NULL) { 
	cout<<"Error in opening the transactions record on the server side"<<endl;
	return -1;
	}
	
	char buffer[1000];
	
	int j =0 , i = 0;
	char *temp;
	
	
	while(fgets(buffer,1000,fp)!= NULL){
		i = 0;
		
		//lets take each string and divide into words by considering space as a separator
		//and store them in the respective variables of the data..acc..name..balance
		
		temp = strtok(buffer, " ");
		if(temp!=NULL && temp!="0") {
		while( temp!= NULL && temp!="0"){
		
		//account
		if(i == 0) {
		data[j].account_no = atoi(temp);
	}	
		//name
		else if(i == 1) {
		strcpy(data[j].name,temp);
	}
		//balance
	else if(i == 2) {
		data[j].balance = atoi(temp);
	}
		i++;
	
	temp =  strtok(NULL," ");
	}
		j++;
	
	}
	//if the line is either empty of consists of 0's then just decrease
	//the number of records that were updated
	else{
		j--;
	}
    }
	
	//close the file pointer
	fclose(fp);
	return j;

}


int main(int args, char* argc[]){

	int server_socket,client_socket,addr_size;
	SA_IN server_addr, client_addr;
	
	//checking the path of the file
	FILE *temp;
	
	char *path = argc[1];
	temp = fopen(path,"r");
	
	if(temp == NULL)
	{
		cout<<"Incorrect path/file name"<<endl<<args;
		fclose(temp);
		return 0;
	}
	
	fclose(temp);
	
	//create a thread pool
	for(int i = 0; i<THREAD_POOL;i++){
		pthread_create(&thread_box[i],NULL,thread_func,(void*)path);
	}

	error_check((server_socket = socket(AF_INET,SOCK_STREAM,0)),"Failed");

	//address struct
	server_addr.sin_family =  AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVERPORT);

	error_check(bind(server_socket,(SA*)&server_addr,sizeof(server_addr)),"Failed at bind");

	error_check(listen(server_socket,SERVER_BACKLOG),"Failed at listen");

	while(true){
	
		cout<<"Waiting for connectionss..."<<endl;
		addr_size = sizeof(SA_IN);
		error_check(client_socket = 
				accept(server_socket,(SA*)&client_addr,
					(socklen_t*)&addr_size),"accept failed");
		cout<<"Connected"<<endl;
		
		int *pclient = (int*)malloc(sizeof(int));
		*pclient = client_socket;
		
		//the following block related to thread pool is referred from Jacob Sorber's tutorial
		//multithreading
		
		//as soon as a connection is established push it into the queue
		//applied the locks so that dequeue and enqueue doesnt happen at the same time
		//and dont result in invalid pointers
		pthread_mutex_lock(&mutex_thread);
		enq(pclient);
		pthread_cond_signal(&condition_var);
		pthread_mutex_unlock(&mutex_thread);
		
				
			
	}
	
	return 0;
}

//a function to print error msgs if any
int error_check(int exp, const char *msg) {

	if(exp == SOCKETERROR){
		perror(msg);
		exit(1);
	}

	return exp; 
}


void *thread_func (void *arg) {
	char* temp_path = (char*) arg;
	while(true) {
	int *pclient;
	pthread_mutex_lock(&mutex_thread);
	if((pclient = dequ()) == NULL){
	
	//wait until a u get a thread enqueued and if any thread is available pull it off and deal with it
	pthread_cond_wait(&condition_var,&mutex_thread);
	
	pclient = dequ();
	}
	pthread_mutex_unlock(&mutex_thread);
	

	if(pclient!=NULL) {
	//we have a connection
	
		handle_connection(pclient,temp_path);
	}
	/*else{
	sleep(0.5);
	}*/
	}
}

void handle_connection(int* p_client_socket, char* path) {
	
	//cout<<"Inside handle connection function "<<*p_client_socket<<endl;//<<"also this"<<data[0].balance;
	int client_socket = *p_client_socket;
	
	//lets free the pointer	
	free(p_client_socket);
	
	//variables for messages that are to be sent and received
	char buffer[BUFSIZE];
	bzero(buffer,BUFSIZE);
	size_t bytes_read;
	int msgsize = 0 ;
	char actualpath[PATH_MAX+1];
	char msg[1000];
	strcpy(msg,"Tx is successful");
	string line;
	int updated;
	
	stringstream account_stream,name_stream,balance_stream;
	
	//cout<<"Path is"<<path<<endl;
	//calling the loaddata
	got.lock();
	updated = loadthedata(path);
	got.unlock();
	
	if(updated==-1) {
		exit(-1);
		}
	if(recv(client_socket , buffer , BUFSIZE , 0) < 0 ) {
		cout<<"Message was not successfully received by the server"<<endl;
	}
	else{
	cout<<"----------------"<<endl;
	cout<<"Data received from the client is - "<<buffer<<endl;
	}
	
	error_check(bytes_read,"rec error");
	buffer[msgsize -1] - 0;

	fflush(stdout);
	
	char *account_str,*name,*temp,*time_stamp ;
	int amount , type;
	
	int i = 0;
	
	//now as we have the data split it based on the space
	//and assign it to respective fields
	
	temp = strtok(buffer," ");
	while(temp!=NULL){
	
	if(i == 0) {
		time_stamp = temp;
	}
	
	else if(i == 1) {
		account_str = temp;
		//cout<<"The account number is "<<account_str<<endl;
	}
	else if(i == 2) {
		name = temp;
		//cout<<"The account number is "<<name<<endl;
	}
	else if(i ==3) {
		if(*temp == 'c' || *temp =='C') 
			type = 1;
		else if(*temp == 'd' || *temp == 'D')
			type = 2 ;
		else
			type =3;
		//cout<<"The type of tx to be performed is " << type <<endl;
		
	}
	else if(i ==4 ) {
		amount  = atoi(temp);
		//cout<<"the amount requested is " <<amount<<endl;
	}
	i++;
	
	temp =  strtok(NULL," ");
	}
	
	//search for the account in the records
	int g= 0 , flag = 0;
	fstream myfile(path);
	
	cout<<"----Locking the CS Part"<<endl;
	got.lock();
	if(myfile.is_open()){
	for(g = 0 ; g < updated;g++) {
		
			//cout<<"Data account no"<<data[g].account_no<<"gice " <<atoi(account_str)<<endl;
			
			
		if( data[g].account_no!=-1 && data[g].account_no == atoi(account_str)) {
		
			flag =1;
		
			//cout<<"found the record at the line"<<g<<endl;
			
			switch(type) {
			
			case 1 : cout<<"Do withdrawal"<<endl;
			    		if(data[g].balance >= amount ) {
						data[g].balance-=amount;
						cout<<"Withdrawal successful"<<endl;
						strcpy(msg,"Withdrawal is success");
						}
					else{
						cout<<"Insufficient balance"<<endl;
						strcpy(msg,"Insufficient balance");
						}
					break;
			
			case 2 : cout <<"Deposit should be done"<<endl;
				 	
				 	data[g].balance+=amount;
				  	cout<<"Deposit success"<<endl;
				  	strcpy(msg,"Deposit is successful");
				  	break;
				
			default: cout<<"DUde wrong tx" <<endl;
				strcpy(msg,"Tx not supported");	
				break;
			
			}
			
			//cout<<"Writing the data on to the file"<<data[g].account_no<<"--"<<data[g].name<<"--"<<data[g].balance<<"--"<<endl;
			//converting the data into stringstreams and writing them to the file
			account_stream << data[g].account_no;
			name_stream << data[g].name;
			balance_stream << data[g].balance;
			
			//writing the streams into the file
			
			myfile<< account_stream.str() << " " << name_stream.str() << " " << balance_stream.str() << " " <<"\n";
			
			//clearing out the streams
			name_stream.str(std::string());
			balance_stream.str(std::string());
			account_stream.str(std::string());
			cout<<"Sending an info to the client"<<endl;
		}
		
		
		//just write the data into the temp file
		else if(data[g].account_no!=-1 && data[g].name!="" && data[g].account_no!=0){
		
		//have the data converted into string streams so that it can be written on to the file
		account_stream << data[g].account_no;
		name_stream << data[g].name;
		balance_stream << data[g].balance;
		
		myfile << account_stream.str() << " " << name_stream.str() << " " << balance_stream.str() << " " <<"\n";
		
		//clearing out the streams
		name_stream.str(std::string());
		balance_stream.str(std::string());
		account_stream.str(std::string());	
		}
		}
		}	
	myfile.close();
	
	//unlock the CS part
	
	got.unlock();
	cout<<"Unlocked the CS part------"<<endl;
	
	//if there is no matching accountnumber
	if(g == updated && flag ==0) {
		cout<<"Record not found"<<endl;
		strcpy(msg,"Record not found");
	}
	
	//sending an ack to the client based on the tx req
	send(client_socket,msg,strlen(msg),0);
	
	//closing the socket
	close(client_socket);
	
	//filling out the buffer with zeroes
	bzero(msg,strlen(msg));
	
	cout<<"closing connection"<<endl;
	cout<<"Press ctrl+c to exit from the server if you are done with it"<<endl;
	cout<<"---------------------"<<endl;
				
}
