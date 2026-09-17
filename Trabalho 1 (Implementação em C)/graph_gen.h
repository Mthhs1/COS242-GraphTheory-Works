#ifndef GRAPH_GEN_H
#define GRAPH_GEN_H

#include "node.h"
#include "graph.h" /* MatrixCell: celula da matriz de adjacencia */

/*
graph_gen.py: criacao das duas representacoes a partir do arquivo de
entrada, e conversao para texto.

Em C os vetores nao carregam o proprio tamanho (len(graph) no Python),
por isso as funcoes de criacao devolvem n em *n_out.
*/

Node **create_empty_list_graph(int n);
Node **create_list_graph(const char *archive_directory, int *n_out);
MatrixCell **create_empty_matrix_graph(int n);
MatrixCell **create_matrix_graph(const char *archive_directory, int *n_out);

char *str_adj_list(Node **graph, int n);
char *str_matrix(MatrixCell **graph, int n);

/* em C quem aloca tambem libera (o Python tem coletor de lixo) */
void free_list_graph(Node **graph, int n);
void free_matrix_graph(MatrixCell **graph, int n);

#endif
