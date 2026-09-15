/*
Questao 1 do estudo de caso (secao 4 do enunciado): comparacao da memoria
usada pelas duas representacoes.

Carrega o grafo na representacao escolhida e pausa a execucao para a
medicao MANUAL de memoria (top/pmap no Linux, gerenciador de tarefas no
Windows). Depois de medir, pressione ENTER e o programa encerra.

uso: ./bin/questao1 <grafo.txt> <matriz|lista>
exemplo com o grafo do estudo de caso (raiz do repositorio):
    ./bin/questao1 ../graph123.txt lista
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

    printf("Grafo carregado: %d vertices | representacao: %s de adjacencia\n",
           g->n, adj_list ? "lista" : "matriz");
    printf("Measure memory.\n");
    printf("(pressione ENTER para encerrar)\n");
    fflush(stdout);

    getchar();

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
