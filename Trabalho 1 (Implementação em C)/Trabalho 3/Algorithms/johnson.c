#include <stdlib.h>

#include "johnson.h"
#include "bellman_ford.h"
#include "graph_utils.h"

/*
Algoritmo de Johnson (ver johnson.h): um Bellman-Ford para o potencial h e,
depois, um Dijkstra com heap por fonte sobre os pesos reduzidos. Nada aqui
reescreve um algoritmo: o Bellman-Ford e o mesmo nucleo de bellman_ford.c
(com todos os vertices comecando em 0 e ativos) e o Dijkstra e o mesmo laco
de dijkstra.c, que aplica o potencial ao ler cada aresta - o grafo nao e
copiado nem alterado.
*/

Johnson *johnson_preparar(Graph *g) {
    BellmanFordResult *r = bellman_ford_potencial(g);

    Johnson *j = xmalloc(sizeof(Johnson));
    j->n = g->n;
    j->negative_cycle = r->negative_cycle;
    j->passadas = r->passadas;
    j->h = r->dist; /* o vetor de distancias do potencial vira h */

    free(r->parents);
    free(r);
    return j;
}

DijkstraResult *johnson_fonte(Johnson *j, Graph *g, int fonte) {
    DijkstraResult *r = dijkstra_reduzido(g, fonte, j->h);

    /* d(s, v) = d'(s, v) - h(s) + h(v); a arvore (parents) ja e a dos
       caminhos minimos verdadeiros, porque o potencial nao muda quais
       caminhos sao minimos */
    for (int v = 0; v < g->n; v++) {
        if (r->dist[v] != INFINITO_PESO) {
            r->dist[v] = r->dist[v] - j->h[fonte] + j->h[v];
        }
    }
    return r;
}

void Johnson_free(Johnson *j) {
    if (j == NULL) {
        return;
    }
    free(j->h);
    free(j);
}
