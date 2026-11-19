#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

/*
Parte 2 do enunciado: distancia e caminho minimo em grafos com pesos.

Dijkstra exige pesos NAO NEGATIVOS. Se o grafo tem alguma aresta com peso
negativo, dijkstra devolve NULL e sugere o Bellman-Ford ou o Floyd-Warshall
da Parte 3, que aceitam pesos negativos.

O enunciado pede o algoritmo de duas formas - com vetor e com heap - mas
manda NAO escrever o algoritmo duas vezes: o que muda e so a forma de
GUARDAR as estimativas, atualiza-las e extrair o minimo. Isso esta
abstraido no DistStore (dijkstra.c); o algoritmo em si e escrito uma vez
so e serve para as duas representacoes do grafo, como as buscas da
Parte 1 servem ao iterador de vizinhanca.
*/

typedef enum {
    DIJKSTRA_VETOR, /* estimativas em vetor; minimo por varredura: O(n^2)  */
    DIJKSTRA_HEAP,  /* estimativas em heap com decrease-key: O(m log n)    */
    /* a "saida facil" da dica do professor, so para MEDIR o seu preco: sem
       decrease-key, o vertice e reinserido a cada melhora e as copias velhas
       sao descartadas na extracao. Mesmo resultado, heap de ate ~m entradas */
    DIJKSTRA_HEAP_REINSERCAO
} DijkstraModo;

typedef struct DijkstraResult {
    double *dist; /* dist[v] = INFINITO_PESO se v nao e alcancavel */
    int *parents; /* arvore de caminhos minimos; -1 = raiz ou inalcancavel */
    long heap_pico;      /* maior numero de entradas no heap (0 no vetor)  */
    long extracoes_vazias; /* copias velhas descartadas (so na reinsercao) */
} DijkstraResult;

void DijkstraResult_free(DijkstraResult *result);

/* Dijkstra a partir de `source` (indice 0-based) para todos os vertices. */
DijkstraResult *dijkstra(Graph *g, int source, DijkstraModo modo);

/* Parte 3 (Johnson): Dijkstra com heap sobre os pesos REDUZIDOS
   peso(u,v) + h[u] - h[v], sem recusar pesos negativos - h tem de ser um
   potencial valido (h = bellman_ford_potencial), que deixa todo peso
   reduzido >= 0. dist devolvida e a REDUZIDA: d(s,v) = dist[v] - h[s] + h[v]. */
DijkstraResult *dijkstra_reduzido(Graph *g, int source, const double *h);

/* Caminho minimo da fonte de `result` ate `alvo` (indice 0-based), em ordem
   fonte -> alvo, com o comprimento em *comprimento_out. Devolve NULL (e
   comprimento 0) se alvo nao e alcancavel. Libere com free(). */
int *dijkstra_path(DijkstraResult *result, int alvo, int *comprimento_out);

#endif
