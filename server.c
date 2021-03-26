#include <stdio.h>
#include <stdlib.h>
#define DEFEAULT_DICT "dictionary.txt"
#define DEFAULT_TEST_DICT "testdict.txt"
#include "queue.h"

int main(int argc, char **argv){
    Queue *test;
    test = init();
    char *t = "ltest";
    enqueue(test, t);
    printf("%d", test->length);
}
