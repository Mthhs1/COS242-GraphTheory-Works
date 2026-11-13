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
vale 1. La cos ("v v") e arestas duplicadas sao ignorados/fundidas (grafo
simples); entre duplicatas com pesos diferentes vale a ULTIMA linha do
arquivo, nas duas representacoes.

Parte 3: o parametro `direcionado` (passado pelo usuario, como pede o
enunciado) diz como ler cada linha. No digrafo, a aresta "a b" so existe
de a para b - e "b a", se tambem estiver no arquivo, e uma aresta
DIFERENTE, nao uma duplicata. No grafo nao direcionado, a aresta existe
nas duas direcoes (como nas Partes 1 e 2).

Os pesos ficam nos nos da lista (Node.weight) e, na matriz, numa matriz
double separada (criada so se o arquivo tem algum peso != 1; sem ela,
toda aresta vale 1). weighted_out/negative_out informam se o arquivo tem
pesos != 1 e se tem peso negativo (Dijkstra recusa o segundo).
*/

Node **create_empty_list_graph(int n);
Node **create_list_graph(const char *archive_directory, int *n_out,
                         bool *weighted_out, bool *negative_out,
                         bool direcionado);
MatrixCell **create_empty_matrix_graph(int n);
MatrixCell **create_matrix_graph(const char *archive_directory, int *n_out,
                                 double ***weights_out, bool *weighted_out,
                                 bool *negative_out, bool direcionado);

/*
Parte 3: o transposto de uma representacao JA construida (as arestas
invertidas), para o Graph_reverse. Na lista, o mesmo bloco unico de nos
das duas passadas; como a aresta v->u vira u->v e v e percorrido em ordem
crescente, cada trecho nasce ordenado sem qsort. Na matriz, a matriz
espelhada (e os pesos junto, quando existem: *transpose_weights_out fica
NULL se weights for NULL).
*/
Node **transpose_list_graph(Node **graph, int n);
MatrixCell **transpose_matrix_graph(MatrixCell **graph, int n, double **weights,
                                    double ***transpose_weights_out);

char *str_adj_list(Node **graph, int n);
char *str_matrix(MatrixCell **graph, int n);

/* em C quem aloca tambem libera (o Python tem coletor de lixo) */
void free_list_graph(Node **graph, int n);
void free_matrix_graph(MatrixCell **graph, int n);
void free_matrix_weights(double **weights, int n);

#endif
