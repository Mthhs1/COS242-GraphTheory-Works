#include <stdio.h>
#include <stdlib.h>

#include "my_queue.h"

void Queue_init(Queue *queue) {
    queue->head = NULL;
    queue->tail = NULL;
}

void Queue_enqueue(Queue *queue, int item) {
    Node *node = Node_create(item, NULL);

    if (queue->head == NULL) {
        queue->head = node;
        queue->tail = node;
        return;
    }

    queue->tail->next = node;
    queue->tail = node;
}

int Queue_dequeue(Queue *queue) {
    if (Queue_isEmpty(queue)) {
        return -1; /* None no Python */
    }

    Node *dequeued = queue->head;
    queue->head = queue->head->next;
    int value = dequeued->value;
    free(dequeued); /* em C o no removido precisa ser liberado */

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    return value;
}

bool Queue_isEmpty(Queue *queue) {
    return queue->head == NULL;
}

int Queue_length(Queue *queue) {
    int n = 0;

    if (Queue_isEmpty(queue)) {
        return n;
    }

    Node *actual = queue->head;

    while (actual != NULL) {
        n += 1;
        actual = actual->next;
    }

    return n;
}

int Queue_front(Queue *queue) {
    if (Queue_isEmpty(queue)) {
        return -1; /* None no Python */
    }
    return queue->head->value;
}

#ifdef MY_QUEUE_MAIN
/* if __name__ == "__main__" do my_queue.py */
int main(void) {
    Queue queue1;
    Queue_init(&queue1);
    Queue_enqueue(&queue1, 1);
    Queue_enqueue(&queue1, 2);
    printf("%d\n", Queue_dequeue(&queue1));
    printf("%s\n", Queue_isEmpty(&queue1) ? "True" : "False");
    printf("%d\n", Queue_length(&queue1));
    printf("%d\n", Queue_front(&queue1));
    return 0;
}
#endif
