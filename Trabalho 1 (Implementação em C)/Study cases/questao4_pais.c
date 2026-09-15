/*
Questao 4 do estudo de caso (secao 4 do enunciado): pai dos vertices 10,
20 e 30 nas arvores geradoras induzidas pela BFS e pela DFS, com buscas
iniciadas nos vertices 1, 2 e 3.

uso: ./bin/questao4 <grafo.txt> <matriz|lista>
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
    printf("pai dos vertices 10, 20 e 30 ('-' = fora da componente da raiz)\n\n");
    printf("%-6s %-6s %8s %8s %8s\n", "busca", "raiz", "pai(10)", "pai(20)", "pai(30)");

    for (int use_dfs = 0; use_dfs <= 1; use_dfs++) {
        for (int raiz = 1; raiz <= 3; raiz++) {
            GraphSearch *busca;
            if (use_dfs) {
                busca = Graph_DFS(g, raiz, 1);
            } else {
                busca = Graph_BFS(g, raiz, 1, 0);
            }

            printf("%-6s %-6d", use_dfs ? "DFS" : "BFS", raiz);
            for (int v = 10; v <= 30; v += 10) {
                int pai = busca->parents[v - 1];
                if (pai == -1) {
                    printf(" %8s", "-");
                } else {
                    printf(" %8d", pai + 1);
                }
            }
            printf("\n");

            GraphSearch_free(busca);
        }
    }

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
