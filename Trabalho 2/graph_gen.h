#ifndef GRAPH_GEN_H
#define GRAPH_GEN_H

#include "graph.h" /* MatrixCell e bool: os prototipos abaixo usam os dois */

/*
graph_gen.py: criacao das duas representacoes a partir do arquivo de
entrada, e conversao para texto.

Em C os vetores nao carregam o proprio tamanho (len(graph) no Python),
por isso as funcoes de criacao devolvem n em *n_out.

Formato do arquivo (Parte 2): a primeira linha tem o numero de vertices;
cada linha seguinte e uma aresta "a b" ou "a b peso" (peso = numero em
ponto flutuante, podendo ser negativo). Sem a terceira coluna, a aresta
vale 1. La cos ("v v") e arestas duplicadas sao ignorados/fundidos (grafo
simples); entre duplicatas com pesos diferentes vale a ULTIMA linha do
arquivo, nas duas representacoes.

Os pesos ficam nos nos da lista (Node.weight) e, na matriz, numa matriz
double separada (criada so se o arquivo tem algum peso != 1; sem ela,
toda aresta vale 1). weighted_out/negative_out informam se o arquivo tem
pesos != 1 e se tem peso negativo (Dijkstra recusa o segundo).
*/

Node **create_empty_list_graph(int n);
Node **create_list_graph(const char *archive_directory, int *n_out,
                         bool *weighted_out, bool *negative_out);
MatrixCell **create_empty_matrix_graph(int n);
MatrixCell **create_matrix_graph(const char *archive_directory, int *n_out,
                                 double ***weights_out, bool *weighted_out,
                                 bool *negative_out);

char *str_adj_list(Node **graph, int n);
char *str_matrix(MatrixCell **graph, int n);

/* em C quem aloca tambem libera (o Python tem coletor de lixo) */
void free_list_graph(Node **graph, int n);
void free_matrix_graph(MatrixCell **graph, int n);
void free_matrix_weights(double **weights, int n);

#endif
