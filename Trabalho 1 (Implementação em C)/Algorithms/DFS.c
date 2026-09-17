#include <stdlib.h>

#include "DFS.h"
#include "my_stack.h"
#include "graph_utils.h"

GraphSearch *DFS_adj_list(Node **graph, int n, int start_node) {
    Stack stack;
    Stack_init(&stack);
    int *vector = xcalloc((size_t)n, sizeof(int));

    /* vizinhos ainda nao visitados do vertice atual, para empilhar na ordem
       inversa (a lista e simplesmente encadeada e nao se percorre de tras) */
    int *pendentes = NULL;
    int pendentes_cap = 0;

    int *result_discovered = xmalloc((size_t)n * sizeof(int));
    int count = 0;

    int *parents = xmalloc((size_t)n * sizeof(int));
    int *levels = xmalloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parents[i] = -1; /* None no Python */
        levels[i] = -1;
    }
    levels[start_node] = 0;

    Stack_push(&stack, start_node);

    while (!Stack_isEmpty(&stack)) {

        int current_node = Stack_pop(&stack);

        if (vector[current_node] == 0) {
            result_discovered[count] = current_node; /* append */
            count += 1;
            vector[current_node] = 1;

            /* A lista esta em ordem crescente. Para o MENOR vizinho sair
               primeiro da pilha (LIFO), empilhamos de tras para frente - o
               mesmo que a DFS_matrix faz com o for de n-1 ate 0. */
            int total = 0;
            Node *neighbor = graph[current_node]->next;

            while (neighbor != NULL) {
                if (vector[neighbor->value] == 0) {
                    if (total == pendentes_cap) {
                        pendentes_cap = (pendentes_cap == 0) ? 16 : pendentes_cap * 2;
                        pendentes = xrealloc(pendentes, (size_t)pendentes_cap * sizeof(int));
                    }
                    pendentes[total] = neighbor->value;
                    total += 1;
                }
                neighbor = neighbor->next;
            }

            for (int k = total - 1; k >= 0; k--) {
                Stack_push(&stack, pendentes[k]);
                parents[pendentes[k]] = current_node;
                levels[pendentes[k]] = levels[current_node] + 1;
            }
        }
    }

    /* em C os nos restantes da pilha precisam ser liberados
       (o Python tem coletor de lixo) */
    while (!Stack_isEmpty(&stack)) {
        Stack_pop(&stack);
    }
    free(vector);
    free(pendentes);

    GraphSearch *search = xmalloc(sizeof(GraphSearch));
    search->result_discovered = result_discovered;
    search->count = count;
    search->parents = parents;
    search->levels = levels;
    return search;
}

GraphSearch *DFS_matrix(int **graph, int n, int start_node) {
    Stack stack;
    Stack_init(&stack);
    int *vector = xcalloc((size_t)n, sizeof(int));

    int *result_discovered = xmalloc((size_t)n * sizeof(int));
    int count = 0;

    int *parents = xmalloc((size_t)n * sizeof(int));
    int *levels = xmalloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parents[i] = -1; /* None no Python */
        levels[i] = -1;
    }
    levels[start_node] = 0;

    Stack_push(&stack, start_node);

    while (!Stack_isEmpty(&stack)) {

        int current_node = Stack_pop(&stack);

        if (vector[current_node] == 0) {
            result_discovered[count] = current_node; /* append */
            count += 1;
            vector[current_node] = 1;

            /* Fazemos o Loop de tras para frente para que o no com menor valor
               seja descoberto primeiro, ja que a pilha e LIFO */
            for (int neighbor = n - 1; neighbor >= 0; neighbor--) {
                if (graph[current_node][neighbor] == 1 && vector[neighbor] == 0) {
                    Stack_push(&stack, neighbor);
                    parents[neighbor] = current_node;
                    levels[neighbor] = levels[current_node] + 1;
                }
            }
        }
    }

    while (!Stack_isEmpty(&stack)) {
        Stack_pop(&stack);
    }
    free(vector);

    GraphSearch *search = xmalloc(sizeof(GraphSearch));
    search->result_discovered = result_discovered;
    search->count = count;
    search->parents = parents;
    search->levels = levels;
    return search;
}
