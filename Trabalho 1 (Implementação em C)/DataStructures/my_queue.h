#ifndef MY_QUEUE_H
#define MY_QUEUE_H

#include <stdbool.h>

#include "node.h"

/* my_queue.py: fila encadeada de inteiros */

typedef struct Queue {
    Node *head;
    Node *tail;
} Queue;

/* queue = Queue() no Python */
void Queue_init(Queue *queue);

void Queue_enqueue(Queue *queue, int item);
int Queue_dequeue(Queue *queue);
bool Queue_isEmpty(Queue *queue);
int Queue_length(Queue *queue);
int Queue_front(Queue *queue);

#endif
