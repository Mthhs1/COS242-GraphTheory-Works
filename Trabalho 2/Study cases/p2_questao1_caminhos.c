/*
Questao 1 da Parte 2 (estudos de caso): distancia e caminho minimo entre
o vertice 10 e os vertices 20, 30, 40, 50 e 60.

uso: ./bin/p2_questao1 <grafo.txt> <matriz|lista> [vetor|heap]
modo padrao: heap (mais rapido; as distancias sao identicas no vetor).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graph.h"
#include "dijkstra.h"

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <matriz|lista> [vetor|heap]\n", programa);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
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

    DijkstraModo modo = DIJKSTRA_HEAP;
    if (argc >= 4) {
        if (strcmp(argv[3], "vetor") == 0) {
            modo = DIJKSTRA_VETOR;
        } else if (strcmp(argv[3], "heap") == 0) {
            modo = DIJKSTRA_HEAP;
        } else {
            fprintf(stderr, "modo invalido: %s (use vetor ou heap)\n", argv[3]);
            return EXIT_FAILURE;
        }
    }

    Graph *g = Graph_create(argv[1], adj_list);

    if (g->n < 60) {
        fprintf(stderr, "grafo com %d vertices: nao existem os vertices 10 a 60\n", g->n);
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    printf("grafo: %s | representacao: %s de adjacencia | modo: %s\n",
           argv[1], argv[2], modo == DIJKSTRA_HEAP ? "heap" : "vetor");

    DijkstraResult *r = dijkstra(g, 10 - 1, modo);
    if (r == NULL) {
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    printf("%-6s %14s  %s\n", "alvo", "distancia", "caminho");
    for (int alvo = 20; alvo <= 60; alvo += 10) {
        if (r->dist[alvo - 1] == INFINITO_PESO) {
            printf("%-6d %14s  %s\n", alvo, "infinito", "sem caminho (outra componente)");
            continue;
        }

        int comprimento;
        int *caminho = dijkstra_path(r, alvo - 1, &comprimento);

        char texto_caminho[4096];
        texto_caminho[0] = '\0';
        for (int i = 0; i < comprimento; i++) {
            char passo[16];
            snprintf(passo, sizeof passo, (i == 0) ? "%d" : " -> %d", caminho[i] + 1);
            strcat(texto_caminho, passo);
        }
        printf("%-6d %14g  %s\n", alvo, r->dist[alvo - 1], texto_caminho);

        free(caminho);
    }

    DijkstraResult_free(r);
    Graph_destroy(g);
    return EXIT_SUCCESS;
}
