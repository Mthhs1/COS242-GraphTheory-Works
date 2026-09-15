/*
Questao 7 do estudo de caso (secao 4 do enunciado): diametro do grafo.

  exato       - BFS a partir de cada vertice: O(n * (n + m)). Preciso, mas
                inviavel em grafos muito grandes.
  aproximado  - dupla varredura por componente (2 BFS cada): rapido, e
                devolve uma cota INFERIOR do diametro.

O modo tem padrao "aproximado", seguro para qualquer tamanho de grafo.
Para grafos pequenos, use "ambos" para comparar as duas respostas.

uso: ./bin/questao7 <grafo.txt> <matriz|lista> [exato|aproximado|ambos]
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "graph.h"

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <matriz|lista> [exato|aproximado|ambos]\n",
            programa);
    exit(EXIT_FAILURE);
}

static double tempo_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
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

    const char *modo = (argc >= 4) ? argv[3] : "aproximado";
    bool exato = strcmp(modo, "exato") == 0 || strcmp(modo, "ambos") == 0;
    bool aproximado = strcmp(modo, "aproximado") == 0 || strcmp(modo, "ambos") == 0;
    if (!exato && !aproximado) {
        fprintf(stderr, "modo invalido: %s (use exato, aproximado ou ambos)\n", modo);
        return EXIT_FAILURE;
    }

    Graph *g = Graph_create(argv[1], adj_list);

    printf("grafo: %s | representacao: %s de adjacencia | modo: %s\n",
           argv[1], argv[2], modo);

    if (exato) {
        double t0 = tempo_ms();
        int d = Graph_diameter(g, 0);
        double t1 = tempo_ms();
        printf("diametro exato      = %d  (%.1f ms)\n", d, t1 - t0);
    }

    if (aproximado) {
        double t0 = tempo_ms();
        int d = Graph_diameter(g, 1);
        double t1 = tempo_ms();
        printf("diametro aproximado = %d  (%.1f ms)  (cota inferior)\n", d, t1 - t0);
    }

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
