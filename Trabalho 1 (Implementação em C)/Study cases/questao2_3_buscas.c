/*
Questoes 2 e 3 do estudo de caso (secao 4 do enunciado): tempo medio de
execucao de buscas em largura (Q2) e em profundidade (Q3), iniciando em
vertices distintos, para cada representacao do grafo.

Conforme o enunciado, o cronometro cobre APENAS o algoritmo: o tempo de
leitura do grafo do disco e de escrita de resultados fica fora da medida.

uso: ./bin/questao2_3 <grafo.txt> <matriz|lista> [n_buscas]
n_buscas tem padrao 100, como pede o enunciado.
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "graph.h"

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <matriz|lista> [n_buscas]\n", programa);
    exit(EXIT_FAILURE);
}

static double tempo_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

/* vertice de partida (rotulo 1-based) do i-esimo teste: vertices
   distintos, espalhados uniformemente pelo intervalo [1, n] */
static int vertice_inicio(int i, int total, int n) {
    return 1 + (int)((long long)i * n / total);
}

typedef struct {
    double soma;
    double minimo;
    double maximo;
} Estatisticas;

static void acumular(Estatisticas *e, double duracao) {
    e->soma += duracao;
    if (e->minimo < 0 || duracao < e->minimo) {
        e->minimo = duracao;
    }
    if (duracao > e->maximo) {
        e->maximo = duracao;
    }
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

    int n_buscas = 100;
    if (argc >= 4) {
        n_buscas = atoi(argv[3]);
        if (n_buscas <= 0) {
            fprintf(stderr, "n_buscas invalido: %s\n", argv[3]);
            return EXIT_FAILURE;
        }
    }

    Graph *g = Graph_create(argv[1], adj_list);

    if (g->n < n_buscas) {
        n_buscas = g->n; /* grafos pequenos: usa todos os vertices */
    }
    if (n_buscas <= 0) {
        fprintf(stderr, "grafo sem vertices\n");
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    printf("grafo: %s | representacao: %s de adjacencia | %d buscas por tipo\n",
           argv[1], argv[2], n_buscas);

    Estatisticas bfs = {0.0, -1.0, -1.0};
    Estatisticas dfs = {0.0, -1.0, -1.0};

    for (int i = 0; i < n_buscas; i++) {
        int inicio = vertice_inicio(i, n_buscas, g->n);

        double t0 = tempo_ms();
        GraphSearch *busca = Graph_BFS(g, inicio, 0, 0);
        double t1 = tempo_ms();
        GraphSearch_free(busca);
        acumular(&bfs, t1 - t0);

        double t2 = tempo_ms();
        GraphSearch *profundidade = Graph_DFS(g, inicio, 0);
        double t3 = tempo_ms();
        GraphSearch_free(profundidade);
        acumular(&dfs, t3 - t2);
    }

    printf("BFS  media: %.4f ms (min %.4f, max %.4f)\n",
           bfs.soma / n_buscas, bfs.minimo, bfs.maximo);
    printf("DFS  media: %.4f ms (min %.4f, max %.4f)\n",
           dfs.soma / n_buscas, dfs.minimo, dfs.maximo);

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
