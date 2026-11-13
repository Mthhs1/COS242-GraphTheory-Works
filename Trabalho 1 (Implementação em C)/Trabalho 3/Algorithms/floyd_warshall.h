#ifndef FLOYD_WARSHALL_H
#define FLOYD_WARSHALL_H

#include "graph.h"

/*
Parte 3 do enunciado: distancia e caminho minimo entre TODOS OS PARES de
vertices, pelo Floyd-Warshall - e se o grafo tem ciclo negativo.

A programacao dinamica "obvia" guardaria D[k][i][j], a distancia minima
de i a j usando so os intermediarios {0, ..., k-1}, para todos os k - um
tensor n^3. A otimizacao de memoria discutida em aula: a passada k so le
a passada k-1, e cada linha D[k-1][k] / coluna D[k-1][*k] NAO muda na
passada k - entao uma UNICA matriz dist[i][j], atualizada no lugar com o
laco k mais externo, faz o mesmo servico com O(n^2) de memoria (n^3 em
tempo). O pred[i][j], que reconstrui os caminhos, tambem e uma unica
matriz n^2.

A deteccao de ciclo negativo: terminado o algoritmo, se algum
dist[i][i] < 0, existe um ciclo negativo ATRAVESSANDO i (dist[i][i] e o
menor passeio fechado por i; um passeio fechado negativo sempre contem
um ciclo negativo). E uma resposta GLOBAL - qualquer ciclo negativo do
grafo aparece em algum dist[i][i] - ao contrario do Bellman-Ford, que so
ve os ciclos que a sua fonte alcanca. Com negative_cycle == true, os
dist/parents que dependem de ciclos nao sao confiaveis.

A arvore de caminhos minimos A PARTIR DE CADA vertice: pred e a matriz de
predecessores - pred[i][j] e o vertice antes de j no caminho minimo i->j
(-1 se nao ha aresta no caminho, ou se j nao e alcancavel de i). A LINHA
i de pred e o vetor de pais da arvore de caminhos minimos enraizada em
i: pred[i][j] = pai de j na arvore. (No Bellman-Ford "de todos para um"
a arvore aponta para o alvo; aqui cada linha nasce apontando para a
raiz i, como no Dijkstra.)
*/

typedef struct FloydWarshallResult {
    int n;                /* ordem das matrizes */
    double **dist;        /* n x n; dist[i][j] = INFINITO_PESO se j nao e
                             alcancavel a partir de i; dist[i][i] = 0 */
    int **pred;           /* n x n; pred[i][j] = predecessor de j no caminho
                             i -> j; -1 = sem aresta/no caminho */
    bool negative_cycle;  /* true se dist[i][i] < 0 para algum i */
} FloydWarshallResult;

void FloydWarshallResult_free(FloydWarshallResult *result);

/* Distancias e arvores entre todos os pares. O(n^2) de memoria (a
   otimizacao), O(n^3) de tempo. */
FloydWarshallResult *floyd_warshall(Graph *g);

/* Caminho minimo origem -> destino (indices 0-based), em ordem origem ->
   destino, com o comprimento em *comprimento_out. NULL (e 0) se o destino
   nao e alcancavel - ou se a arvore de predecessores esta corrompida por
   ciclo negativo (mais de n passos). Libere com free(). */
int *floyd_warshall_path(FloydWarshallResult *result, int origem, int destino,
                         int *comprimento_out);

#endif
