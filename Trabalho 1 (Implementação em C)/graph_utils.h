#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include <stddef.h>

#include "graph.h"

/*
Utilitarios compartilhados pelos modulos de componentes, distancia e
estatisticas (graph_utils.py).

A biblioteca suporta as duas representacoes exigidas pelo enunciado
(matriz e lista de adjacencia). Para nao duplicar cada algoritmo, tudo
aqui usa a flag `adj_list` do grafo e neighbors() como unica porta de
acesso a vizinhanca.

No Python neighbors() e um gerador (yield); em C devolve um vetor
alocado com os vizinhos e o tamanho em *count_out (o chamador libera).
*/

int *neighbors(Graph *g, int v, int *count_out);
int degree(Graph *g, int v);
int *all_degrees(Graph *g);
int edge_count(Graph *g);

/* Auxiliares que em C substituem o gerenciamento automatico do Python */
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);

/* Concatenacao estilo `texto += f"..."` do Python; devolve o novo buffer */
char *str_append(char *buffer, const char *format, ...);

#endif
