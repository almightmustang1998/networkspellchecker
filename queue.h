//referenced professors fiore's stack files to model this queue
#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct {
    Node *front;
    Node *back;
    int length;
} Queue;

Queue *init();
void cleanup(Queue *);

void enqueue(Queue *, void *);
void *dequeue(Queue *);
int is_empty(Queue *);

#endif
