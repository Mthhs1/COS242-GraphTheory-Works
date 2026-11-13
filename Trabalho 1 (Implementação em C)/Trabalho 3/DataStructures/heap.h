#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>

/*
Heap binario de MINIMOS de vertices (0..n-1) com chaves double (Parte 2:
guarda as estimativas de distancia do Dijkstra).

O ponto exigido pelo enunciado da Parte 2: um heap cujas chaves possam ser
MODIFICADAS de forma eficiente (decrease-key). O heap guarda, em
posicao[vertice], onde cada vertice esta no array; assim
MinHeap_decrease_key encontra o elemento em O(1) e conserta o heap
subindo em O(log n), sem busca nenhuma.

Cada vertice entra no maximo uma vez (insert de vertice presente e erro
de contrato); por isso os arrays sao alocados com n posicoes no inicio.
*/

typedef struct MinHeap {
    int *vertices;  /* vertices[i] = vertice na posicao i        */
    double *chaves; /* chaves[i]   = chave do vertice na posicao i */
    int *posicao;   /* posicao[v]  = i, ou -1 se v nao esta no heap */
    int tamanho;
    int capacidade; /* n: um vertice por posicao */
} MinHeap;

/* aloca os arrays para ate n vertices distintos (heap vazio) */
void MinHeap_init(MinHeap *heap, int n);

void MinHeap_insert(MinHeap *heap, int vertice, double chave);
bool MinHeap_contains(MinHeap *heap, int vertice);
/* vertice precisa estar no heap e nova_chave <= chave atual */
void MinHeap_decrease_key(MinHeap *heap, int vertice, double nova_chave);

int MinHeap_extract_min(MinHeap *heap); /* vertice de menor chave; -1 se vazio */
bool MinHeap_isEmpty(MinHeap *heap);
void MinHeap_destroy(MinHeap *heap);

#endif
