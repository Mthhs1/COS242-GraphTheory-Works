/*
Questao 5 do estudo de caso (secao 4 do enunciado): distancia entre os
pares de vertices (10,20), (10,30) e (20,30).

uso: ./bin/questao5 <grafo.txt> <matriz|lista>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graph.h"

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <matriz|lista>\n", programa);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        uso(argv[0]);
    }

    bool adj_list;
    if (strcmp(argv[2], "lista") == 0) {
        adj_list = true;
    } else if (strcmp(argv[2], "matriz") == 0) {
        adj_list = false;
    } else {
        fprintf(stderr, "representacao invalida: %s (use matriz ou lista)\n", argv[2]);
        return EXIT_FAILURE;
    }

    Graph *g = Graph_create(argv[1], adj_list);

    if (g->n < 30) {
        fprintf(stderr, "grafo com %d vertices: nao existem os vertices 10, 20 e 30\n", g->n);
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    printf("grafo: %s | representacao: %s de adjacencia\n", argv[1], argv[2]);

    int pares[3][2] = {{10, 20}, {10, 30}, {20, 30}};
    for (int i = 0; i < 3; i++) {
        int u = pares[i][0];
        int v = pares[i][1];
        int d = Graph_distance(g, u, v);
        printf("d(%d,%d) = %d", u, v, d);
        if (d == -1) {
            printf("  (vertices em componentes distintas)");
        }
        printf("\n");
    }

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
