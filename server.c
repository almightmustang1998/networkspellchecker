#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"
#include "open_listenfd.c"
#define DEFAULT_DICT "dictionary.txt"
#define DEFAULT_TEST_DICT "testdict.txt"
#define DEFAULT_PORT 1029
#define NUM_WORKERS 8

int connectionPort;
char *dictionary;

pthread_mutex_t workMutex, logMutex;
pthread_cond_t workAvailable, logAvailable;

int main(int argc, char **argv){
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
    pthread_mutex_init(&workMutex, NULL);
    pthread_cond_init(&workAvailable, NULL);

    pthread_mutex_init(&logMutex, NULL);
    pthread_cond_init(&logAvailable, NULL);

    //sockaddr_in holds information about the user connection.
	//We don't need it, but it needs to be passed into accept().
	struct sockaddr_in client;
	int clientLen = sizeof(client);
	
	int connectionSocket, clientSocket, bytesReturned;
	char recvBuffer[BUF_LEN];
	recvBuffer[0] = '\0';

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

   /* Queue *test;
    test = init();
    char *t = "ltest";
    enqueue(test, t);
    printf("%d", test->length);
*/

	//We can't use ports below 1024 and ports above 65535 don't exist.
	if(connectionPort < 1024 || connectionPort > 65535){
		printf("Port number is either too low(below 1024), or too high(above 65535).\n");
		return -1;
	}

    while(1){
    //Does all the hard work for us.
	connectionSocket = open_listenfd(connectionPort);
	if(connectionSocket == -1){
		printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
		return -1;
	}
    }
}
//NEED TO finish reading in dictionary
void *worker_thread(void *args){

}

void *log_thread(void *args){
    
}