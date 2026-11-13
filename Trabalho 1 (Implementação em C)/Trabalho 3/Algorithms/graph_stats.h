#ifndef GRAPH_STATS_H
#define GRAPH_STATS_H

#include "graph.h"

/*
Requisito 2 do enunciado: arquivo de saida com as informacoes do grafo.

Gera numero de vertices, numero de arestas, grau minimo, maximo, medio e
mediana de grau, seguidos das informacoes das componentes conexas.
*/

/* o dict devolvido por graph_stats() no Python */
typedef struct GraphStats {
    int vertices;
    int arestas;
    int grau_minimo;
    int grau_maximo;
    double grau_medio;
    double mediana_grau;
} GraphStats;

double median(int *values, int len);
GraphStats graph_stats(Graph *g);
char *stats_text(Graph *g);
char *write_output(Graph *g, const char *output_path);
char *write_search_tree(int *parents, int *levels, int n, const char *output_path, int root);

#endif
