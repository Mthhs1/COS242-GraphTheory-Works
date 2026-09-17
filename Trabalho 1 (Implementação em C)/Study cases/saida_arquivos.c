/*
Requisitos 2 e 4 do enunciado aplicados aos grafos do estudo de caso:
estatisticas do grafo e arquivos de saida.

Imprime n, m, grau minimo, maximo, medio e mediana, e grava na pasta
indicada:
  <grafo>_saida.txt       requisito 2 (estatisticas + componentes conexas)
  <grafo>_arvore_bfs.txt  requisito 4 (pai e nivel de cada vertice, BFS a partir de 1)
  <grafo>_arvore_dfs.txt  requisito 4 (idem, DFS a partir de 1)

Os tempos impressos cobrem a geracao e a escrita de cada arquivo (nao a
leitura do grafo).

uso: ./bin/saida <grafo.txt> <matriz|lista> <pasta_de_saida>
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "graph.h"
#include "graph_stats.h"

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <matriz|lista> <pasta_de_saida>\n", programa);
    exit(EXIT_FAILURE);
}

static double tempo_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

/* "Grafos/grafo_1.txt" -> "grafo_1" */
static void nome_base(const char *caminho, char *destino, size_t tamanho) {
    const char *barra = strrchr(caminho, '/');
    const char *inicio = (barra != NULL) ? barra + 1 : caminho;
    snprintf(destino, tamanho, "%s", inicio);
    char *ponto = strrchr(destino, '.');
    if (ponto != NULL) {
        *ponto = '\0';
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
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

    char base[256];
    nome_base(argv[1], base, sizeof base);
    char caminho[1024];

    Graph *g = Graph_create(argv[1], adj_list);
    printf("grafo: %s | representacao: %s de adjacencia\n", argv[1], argv[2]);

    GraphStats *s = Graph_stats(g);
    printf("Numero de vertices: %d\n", s->vertices);
    printf("Numero de arestas: %d\n", s->arestas);
    printf("Grau minimo: %d\n", s->grau_minimo);
    printf("Grau maximo: %d\n", s->grau_maximo);
    printf("Grau medio: %.4f\n", s->grau_medio);
    printf("Mediana de grau: %.1f\n", s->mediana_grau);
    free(s);

    snprintf(caminho, sizeof caminho, "%s/%s_saida.txt", argv[3], base);
    double t0 = tempo_ms();
    free(Graph_write_output(g, caminho));
    printf("arquivo de saida (requisito 2): %s  (%.1f ms)\n", caminho, tempo_ms() - t0);

    for (int use_dfs = 0; use_dfs <= 1; use_dfs++) {
        snprintf(caminho, sizeof caminho, "%s/%s_arvore_%s.txt", argv[3], base,
                 use_dfs ? "dfs" : "bfs");
        double t1 = tempo_ms();
        free(Graph_write_search_tree(g, caminho, 1, use_dfs));
        printf("arvore %s a partir de 1 (requisito 4): %s  (%.1f ms)\n",
               use_dfs ? "DFS" : "BFS", caminho, tempo_ms() - t1);
    }

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
