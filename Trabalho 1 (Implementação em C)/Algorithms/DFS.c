#include <stdlib.h>

#include "DFS.h"
#include "my_stack.h"
#include "graph_utils.h"

GraphSearch *DFS(Graph *g, int start_node) {
    int n = g->n;

    Stack stack;
    Stack_init(&stack);
    int *vector = xcalloc((size_t)n, sizeof(int));

    /* vizinhos ainda nao visitados do vertice atual, para empilhar na ordem
       inversa; o vetor e alocado uma vez e reaproveitado em toda a busca */
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
            vector[current_node] = 1; /* marca ao sair da pilha */

            /* O iterador entrega os vizinhos em ordem crescente. Para o MENOR
               sair primeiro da pilha (LIFO), empilhamos de tras para frente. */
            int total = 0;
            int neighbor;
            NeighborIter it = neighbor_iter(g, current_node);
            while (neighbor_next(&it, &neighbor)) {
                if (vector[neighbor] == 0) {
                    if (total == pendentes_cap) {
                        pendentes_cap = (pendentes_cap == 0) ? 16 : pendentes_cap * 2;
                        pendentes = xrealloc(pendentes, (size_t)pendentes_cap * sizeof(int));
                    }
                    pendentes[total] = neighbor;
                    total += 1;
                }
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
