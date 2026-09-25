#include <stdlib.h>

#include "BFS.h"
#include "my_queue.h"
#include "graph_utils.h"

GraphSearch *BFS(Graph *g, int start_node, int target) {
    int n = g->n;

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

        /* vizinhos em ordem crescente nas duas representacoes */
        int neighbor;
        NeighborIter it = neighbor_iter(g, current_node);
        while (neighbor_next(&it, &neighbor)) {
            if (vector[neighbor] == 0) {
                Queue_enqueue(&queue, neighbor);
                vector[neighbor] = 1; /* marca ao entrar na fila */
                parents[neighbor] = current_node;
                levels[neighbor] = levels[current_node] + 1;
            }
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
