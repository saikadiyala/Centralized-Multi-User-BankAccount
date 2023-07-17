#include "../header/mque.h"
#include <stdlib.h>

node_t* head = NULL;
node_t* tail = NULL;

void enq(int *client_sock) {

	node_t *newnode = (node_t*)malloc(sizeof(node_t));
	newnode->client_socket = client_sock;
	newnode->next = NULL;
	if(tail == NULL){
	head = newnode;
	}
	else{
		tail ->next = newnode;
	}
	tail = newnode;
}

//return NULL If the q is empty
//return the pointer to a client socket, if htere is one to get
//

int* dequ() {
	
	if(head == NULL) {
	return NULL;
	}
	else {
	
		int *result = head->client_socket;
		node_t *temp = head;
		head = head ->next;
		if(head ==NULL) tail = NULL;
		free(temp);
		return result;
	
	}

}
