#include <stdlib.h>

#include "BFS.h"
#include "my_queue.h"
#include "graph_utils.h"

GraphSearch *BFS_adj_list(Node **graph, int n, int start_node, int target) {
    Queue queue;
    Queue_init(&queue);
    int *vector = xcalloc((size_t)n, sizeof(int));

    int *parents = xmalloc((size_t)n * sizeof(int));
    int *levels = xmalloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parents[i] = -1; /* None no Python */
        levels[i] = -1;
    }
    levels[start_node] = 0;

    Queue_enqueue(&queue, start_node);
    vector[start_node] = 1;

    int *result_discovered = xmalloc((size_t)n * sizeof(int));
    int count = 0;

    while (!Queue_isEmpty(&queue)) {
        int current_node = Queue_dequeue(&queue);
        result_discovered[count] = current_node; /* append */
        count += 1;

        if (current_node == target) {
            break;
        }

        Node *neighbor = graph[current_node]->next;

        while (neighbor != NULL) {
            if (vector[neighbor->value] == 0) {
                Queue_enqueue(&queue, neighbor->value);
                vector[neighbor->value] = 1;
                parents[neighbor->value] = current_node;
                levels[neighbor->value] = levels[current_node] + 1;
            }
            neighbor = neighbor->next;
        }
    }

    /* em C os nos restantes da fila precisam ser liberados
       (o Python tem coletor de lixo) */
    while (!Queue_isEmpty(&queue)) {
        Queue_dequeue(&queue);
    }
    free(vector);

    GraphSearch *search = xmalloc(sizeof(GraphSearch));
    search->result_discovered = result_discovered;
    search->count = count;
    search->parents = parents;
    search->levels = levels;
    return search;
}

GraphSearch *BFS_matrix(MatrixCell **graph, int n, int start_node, int target) {
    Queue queue;
    Queue_init(&queue);
    int *vector = xcalloc((size_t)n, sizeof(int));

    int *parents = xmalloc((size_t)n * sizeof(int));
    int *levels = xmalloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parents[i] = -1; /* None no Python */
        levels[i] = -1;
    }
    levels[start_node] = 0;

    Queue_enqueue(&queue, start_node);
    vector[start_node] = 1;

    int *result_discovered = xmalloc((size_t)n * sizeof(int));
    int count = 0;

    while (!Queue_isEmpty(&queue)) {
        int current_node = Queue_dequeue(&queue);
        result_discovered[count] = current_node; /* append */
        count += 1;

        if (current_node == target) {
            break;
        }

        for (int neighbor = 0; neighbor < n; neighbor++) {
            if (graph[current_node][neighbor] == 1 && vector[neighbor] == 0) {
                Queue_enqueue(&queue, neighbor);
                vector[neighbor] = 1;
                parents[neighbor] = current_node;
                levels[neighbor] = levels[current_node] + 1;
            }
        }
    }

    while (!Queue_isEmpty(&queue)) {
        Queue_dequeue(&queue);
    }
    free(vector);

    GraphSearch *search = xmalloc(sizeof(GraphSearch));
    search->result_discovered = result_discovered;
    search->count = count;
    search->parents = parents;
    search->levels = levels;
    return search;
}
