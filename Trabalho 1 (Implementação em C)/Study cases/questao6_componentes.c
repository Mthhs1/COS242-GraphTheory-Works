/*
Questao 6 do estudo de caso (secao 4 do enunciado): componentes conexas
do grafo - quantas sao, tamanho da maior e tamanho da menor.

uso: ./bin/questao6 <grafo.txt> <matriz|lista>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graph.h"
#include "components.h"

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

    printf("grafo: %s | representacao: %s de adjacencia\n", argv[1], argv[2]);

    Components comps = connected_components(g);

    printf("Numero de componentes conexas: %d\n", comps.count);
    if (comps.count > 0) {
        printf("Maior componente: %d vertices\n", comps.sizes[0]);
        printf("Menor componente: %d vertices\n", comps.sizes[comps.count - 1]);
    }

    Components_free(&comps);
    Graph_destroy(g);
    return EXIT_SUCCESS;
}
