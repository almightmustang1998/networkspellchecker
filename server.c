#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#include "queue.h"
#include "open_listenfd.c"
#define DEFAULT_DICT "dictionary.txt"
#define DEFAULT_TEST_DICT "testdic.txt"
#define DEFAULT_PORT 1029
#define NUM_WORKERS 5
void *worker_thread(void *args);
void *log_thread(void *args);
int spellChecker(char *fileName, char *word);

pthread_mutex_t clientLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clientCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t logCond = PTHREAD_COND_INITIALIZER;

int connectionPort;
char *dictionary;
Queue *clientQueue;
Queue *loggerQueue;

int main(int argc, char **argv){

    clientQueue = init();
    loggerQueue = init();

     //sockaddr_in holds information about the user connection.
	//We don't need it, but it needs to be passed into accept().
	struct sockaddr_in client;
	int clientLen = sizeof(client);
	
	int connectionSocket, clientSocket;

    //create worker and log thread
    pthread_t workers[NUM_WORKERS];
    pthread_t log;

    //create worker threads
    for (int i = 0; i < NUM_WORKERS; i++) {
    pthread_create(&workers[i], NULL, &worker_thread, NULL);
  }
    //create log thread
    pthread_create(&log, NULL, &log_thread, NULL);

   //check program argmuments
   //and adjust ports/dictionary accordingly
   if(argc == 1) {
    connectionPort = DEFAULT_PORT;
    dictionary = DEFAULT_DICT;
  } else if(argc == 2) {
      connectionPort = atoi(argv[1]);
      dictionary = DEFAULT_DICT;
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

    //lock so we can safely utilize the queue without it being corrupted
    pthread_mutex_lock(&clientLock);
    
    //if the queue is full, make client wait in order to avoid deadlock
    while (clientQueue->length==NUM_WORKERS){
    pthread_cond_wait(&clientCond, &clientLock); 
    }
    enqueue(clientQueue, &clientSocket);
    pthread_cond_signal(&clientCond);
    pthread_mutex_unlock(&clientLock);

	}
}

void *worker_thread(void *args){
    int clientSocket;
    int bytesReturned;
    int spellCheck;
	char recvBuffer[BUF_LEN];
	recvBuffer[0] = '\0';
    char* deliminator = " \r\n";
	char* clientMessage = "Hello! Welcome to the Spellchecker!!!\n";
	char* msgRequest = "Send me some text and I'll search through my dictionary to tell you if it is a correctly spelled word!\nSend the escape key to close the connection.\n";
	char* msgPrompt = ">>> ";
    char* ok = " OK\n";
    char* mispelled = " MISSPELLED\n";
	char* msgError = "I didn't get your message. ):\n";
	char* msgClose = "Goodbye!\n";
    char* created = "Worker created!\n";
    char* sleep = "Worker going to sleep\n";
    char* working = "Worker accepted client and started working!\n";

    pthread_mutex_lock(&logLock);
    enqueue(loggerQueue, created);
    pthread_cond_signal(&logCond);
    pthread_mutex_unlock(&logLock);

    while(1){

    pthread_mutex_lock(&clientLock);

    while(is_empty(clientQueue)!=0){
        pthread_mutex_lock(&logLock);
        enqueue(loggerQueue, sleep);
        pthread_cond_signal(&logCond);
        pthread_mutex_unlock(&logLock);
        pthread_cond_wait(&clientCond, &clientLock);
    }
    
    //create temp variable to solve void pointer problem
    int p = *(int*)dequeue(clientQueue);
    clientSocket = p;
    pthread_mutex_unlock(&clientLock);
    
	//send()...sends a message.
	//We specify the socket we want to send, the message and it's length, the
	//last parameter are flags.
    pthread_mutex_lock(&logLock);
    enqueue(loggerQueue, working);
    pthread_cond_signal(&logCond);
    pthread_mutex_unlock(&logLock);
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
			close(clientSocket);
			break;
		}
		else{
        char *response = (char*) malloc(15);
        if(response== NULL){
            printf("Malloc failed\n");
        }
            //needed to elimate new line || space || carriege return
            char* buffer = strtok(recvBuffer, deliminator);
             //if word is in the dictionary
             //echo back okay
            spellCheck = spellChecker(dictionary, buffer);
            //logic for responding to clients
            if (spellCheck == 1){
                response = strcat(buffer, ok);
			    send(clientSocket, response, strlen(response), 0);
            }
            else if (spellCheck ==0){
                response = strcat(buffer, mispelled);
			    send(clientSocket, response, strlen(response), 0);
            }
        pthread_mutex_lock(&logLock);
        enqueue(loggerQueue, response);
        pthread_cond_signal(&logCond);
        pthread_mutex_unlock(&logLock);
		}  
    }
} return NULL;
}

void *log_thread(void *args){
   char *logcreation = "Logger created!\n";
        pthread_mutex_lock(&logLock);
        enqueue(loggerQueue, logcreation);
        pthread_cond_signal(&logCond);
        pthread_mutex_unlock(&logLock);
    while(1){
        //lock logger lock
        pthread_mutex_lock(&logLock);
        //while queue is empty, make it wait
        while(is_empty(loggerQueue)!=0){
        pthread_cond_wait(&logCond, &logLock);
    }
        char* word = (char*)dequeue(loggerQueue);
        printf("%s", word);
        pthread_mutex_unlock(&logLock);
    }

}

//spellchecker function takes a file and word to check if 
//it exists in the file. If it does exist 1 is returned. 
int spellChecker(char *fileName, char *word){
    //holds length of file
    int length = 0; 
    FILE *file;
   
    if((file=fopen(fileName, "r")) == NULL){
       // go through file
       printf("Error opening file\n");
       return -1;
    }
    while(getc(file) != EOF){
        length++;
    }
     rewind(file);

    //allocate memory to store data from file
    char *buffer = (char*) malloc(length+1);

    if(buffer == NULL){
        printf("Malloc failed\n");
        return -1;
    }
    
    while(fgets(buffer, length, file) != NULL){
        strtok(buffer, "\n");
        if (strcmp(buffer, word)==0){
            return 1;
        }
    }     
       //closing file
       fclose(file);
       //deallocating memory 
       free(buffer);
       return 0;
 }