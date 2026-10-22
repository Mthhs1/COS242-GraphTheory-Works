#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <math.h>

#include "graph.h"

/*
Parte 2 do enunciado: distancia e caminho minimo em grafos com pesos.

Dijkstra exige pesos NAO NEGATIVOS. Se o grafo tem alguma aresta com peso
negativo, dijkstra devolve NULL e informa que a biblioteca ainda nao
implementa caminhos minimos com pesos negativos (o enunciado manda
informar, nao resolver).

O enunciado pede o algoritmo de duas formas - com vetor e com heap - mas
manda NAO escrever o algoritmo duas vezes: o que muda e so a forma de
GUARDAR as estimativas, atualiza-las e extrair o minimo. Isso esta
abstraido no DistStore (dijkstra.c); o algoritmo em si e escrito uma vez
so e serve para as duas representacoes do grafo, como as buscas da
Parte 1 servem ao iterador de vizinhanca.
*/

/* distancia "infinita" para vertices nao alcancados. O INFINITO da Parte 1
   (-1) nao serve aqui: em grafo com pesos, -1 pode ser distancia valida. */
#define INFINITO_PESO HUGE_VAL

typedef enum {
    DIJKSTRA_VETOR, /* estimativas em vetor; minimo por varredura: O(n^2)  */
    DIJKSTRA_HEAP   /* estimativas em heap com decrease-key: O(m log n)    */
} DijkstraModo;

typedef struct DijkstraResult {
    double *dist; /* dist[v] = INFINITO_PESO se v nao e alcancavel */
    int *parents; /* arvore de caminhos minimos; -1 = raiz ou inalcancavel */
} DijkstraResult;

void DijkstraResult_free(DijkstraResult *result);

/* Dijkstra a partir de `source` (indice 0-based) para todos os vertices. */
DijkstraResult *dijkstra(Graph *g, int source, DijkstraModo modo);

/* Caminho minimo da fonte de `result` ate `alvo` (indice 0-based), em ordem
   fonte -> alvo, com o comprimento em *comprimento_out. Devolve NULL (e
   comprimento 0) se alvo nao e alcancavel. Libere com free(). */
int *dijkstra_path(DijkstraResult *result, int alvo, int *comprimento_out);

#endif
