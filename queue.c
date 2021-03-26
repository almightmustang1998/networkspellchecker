//defines queue and methods
//defining a node
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

Queue *init(){
    Queue *queue;
    queue = (Queue *)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->back = NULL;
    queue->length = 0;
    return queue;
}

//adds data to queue
void enqueue(Queue *queue, void *data){
    //put data inside of node
    Node *node = (Node*)malloc(sizeof(Node));
    node->next = NULL;
    node->data = data;
    //if the queue is empty, point the front and back to the new node
    if(queue->length == 0){
        queue->front = node;
        queue->back = node;
    }
    //if it is not empty, make sure the node and queue point to the new node
    else {
        queue->back->next = node;
        queue->back = node;
    }
    //increase length
    queue->length++;

}

//removes data from front of queue
void *dequeue(Queue *queue){
    if(queue->length!=0){
        Node *removed = queue->front;
        queue->front = removed->next;
        void *data = removed->data;
        free(removed);
        queue->length--;
        return data;
    }
    else{
        return NULL;
    }
}
//check is queue is empty
int is_empty(Queue *queue){
    return queue->length==0;
}

