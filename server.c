#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#include "queue.h"
#include "open_listenfd.c"
#define DEFAULT_DICT "dictionary.txt"
#define DEFAULT_TEST_DICT "testdict.txt"
#define DEFAULT_PORT 1029
#define NUM_WORKERS 3
void *worker_thread(void *args);
void *log_thread(void *args);

pthread_mutex_t clientLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clientCond = PTHREAD_COND_INITIALIZER;

int connectionPort;
char *dictionary;

Queue *test;

//pthread_mutex_t workMutex, logMutex;
//pthread_cond_t workAvailable, logAvailable;

int main(int argc, char **argv){
    
    test = init();

     //sockaddr_in holds information about the user connection.
	//We don't need it, but it needs to be passed into accept().
	struct sockaddr_in client;
	int clientLen = sizeof(client);
	
	int connectionSocket, clientSocket;
    //init q's

    //create worker and log thread
    pthread_t workers[NUM_WORKERS];
    pthread_t log;

    //create worker threads
    for (int i = 0; i < NUM_WORKERS; i++) {
    pthread_create(&workers[i], NULL, &worker_thread, NULL);
  }
    //create log thread
    pthread_create(&log, NULL, &log_thread, NULL);

    // init mutexes & conditions
 //   pthread_mutex_init(&workMutex, NULL);
 //   pthread_cond_init(&workAvailable, NULL);

 //   pthread_mutex_init(&logMutex, NULL);
 //   pthread_cond_init(&logAvailable, NULL);

   //check program argmuments
   //and adjust ports/dictionary accordingly
   if(argc == 1) {
    connectionPort = DEFAULT_PORT;
    dictionary = DEFAULT_TEST_DICT;
  } else if(argc == 2) {
      connectionPort = atoi(argv[1]);
      dictionary = DEFAULT_TEST_DICT;
  } else {
      connectionPort = atoi(argv[1]);
      dictionary = argv[2];
  }

  //We can't use ports below 1024 and ports above 65535 don't exist.
	if(connectionPort < 1024 || connectionPort > 65535){
		printf("Port number is either too low(below 1024), or too high(above 65535).\n");
		return -1;
	}

    connectionSocket = open_listenfd(connectionPort);

    while(1){

    if((clientSocket = accept(connectionSocket, (struct sockaddr*) &client, (socklen_t *) &clientLen)) == -1){
		printf("Error connecting to client.\n");
		return -1;
	}

	if(connectionSocket == -1){
		printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
		return -1;
	}

    pthread_mutex_lock(&clientLock);
    enqueue(test, &clientSocket);
    pthread_cond_signal(&clientCond);
    pthread_mutex_unlock(&clientLock);

	}
}




void *worker_thread(void *args){
    int clientSocket;
    int bytesReturned;
	char recvBuffer[BUF_LEN];
	recvBuffer[0] = '\0';

	char* clientMessage = "Hello! I hope you can see this.\n";
	char* msgRequest = "Send me some text and I'll respond with something interesting!\nSend the escape key to close the connection.\n";
	char* msgResponse = "I actually don't have anything interesting to say...but I know you sent ";
	char* msgPrompt = ">>>";
	char* msgError = "I didn't get your message. ):\n";
	char* msgClose = "Goodbye!\n";
    
    while(1){
    
    pthread_mutex_lock(&clientLock);
    while(is_empty(test)!=0){
        pthread_cond_wait(&clientCond, &clientLock);
    }

// &clientSocket = *(int*)dequeue(test);     //cannot dereference void pointers

//create temp variable to solve void pointer problem
    int p = *(int*)dequeue(test);
    printf("\n%d", &p);
    clientSocket = p;

    pthread_mutex_unlock(&clientLock);
   
	//send()...sends a message.
	//We specify the socket we want to send, the message and it's length, the
	//last parameter are flags.
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	send(clientSocket, msgRequest, strlen(msgRequest), 0);


	//Begin sending and receiving messages.
	while(1){
		send(clientSocket, msgPrompt, strlen(msgPrompt), 0);
		//recv() will store the message from the user in the buffer, returning
		//how many bytes we received.
		bytesReturned = recv(clientSocket, recvBuffer, BUF_LEN, 0);

		//Check if we got a message, send a message back or quit if the
		//user specified it.
		if(bytesReturned == -1){
			send(clientSocket, msgError, strlen(msgError), 0);
		}
		//'27' is the escape key.
		else if(recvBuffer[0] == 27){
			send(clientSocket, msgClose, strlen(msgClose), 0);
		//	close(clientSocket);
        //need to fix close funct
			break;
		}
		else{
			send(clientSocket, msgResponse, strlen(msgResponse), 0);
			send(clientSocket, recvBuffer, bytesReturned, 0);
			//This line will send it back to the server, it also clears the old buffer
			//fflush(recvBuffer);
		//	write(1, recvBuffer, bytesReturned);
		}
    }
    return NULL;
}
}

void *log_thread(void *args){
        printf("logtest\n");
    return NULL;
}