#include <stdlib.h>

#include "heap.h"
#include "graph_utils.h"

/* troca duas posicoes do array, mantendo posicao[] em dia */
static void trocar(MinHeap *heap, int i, int j) {
    int vi = heap->vertices[i];
    int vj = heap->vertices[j];

    heap->vertices[i] = vj;
    heap->vertices[j] = vi;

    double ci = heap->chaves[i];
    heap->chaves[i] = heap->chaves[j];
    heap->chaves[j] = ci;

    heap->posicao[vi] = j;
    heap->posicao[vj] = i;
}

/* conserta o heap subindo a posicao i (chave menor que a do pai) */
static void subir(MinHeap *heap, int i) {
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (heap->chaves[pai] <= heap->chaves[i]) {
            break;
        }
        trocar(heap, i, pai);
        i = pai;
    }
}

/* conserta o heap descendo a posicao i (chave maior que a de algum filho) */
static void descer(MinHeap *heap, int i) {
    for (;;) {
        int filho = 2 * i + 1;
        if (filho >= heap->tamanho) {
            break;
        }
        if (filho + 1 < heap->tamanho && heap->chaves[filho + 1] < heap->chaves[filho]) {
            filho += 1;
        }
        if (heap->chaves[i] <= heap->chaves[filho]) {
            break;
        }
        trocar(heap, i, filho);
        i = filho;
    }
}

void MinHeap_init(MinHeap *heap, int n) {
    heap->vertices = xmalloc((size_t)n * sizeof(int));
    heap->chaves = xmalloc((size_t)n * sizeof(double));
    heap->posicao = xmalloc((size_t)n * sizeof(int));
    for (int v = 0; v < n; v++) {
        heap->posicao[v] = -1;
    }
    heap->tamanho = 0;
    heap->capacidade = n;
}

void MinHeap_insert(MinHeap *heap, int vertice, double chave) {
    int i = heap->tamanho;
    heap->tamanho += 1;

    heap->vertices[i] = vertice;
    heap->chaves[i] = chave;
    heap->posicao[vertice] = i;

    subir(heap, i);
}

bool MinHeap_contains(MinHeap *heap, int vertice) {
    return heap->posicao[vertice] != -1;
}

void MinHeap_decrease_key(MinHeap *heap, int vertice, double nova_chave) {
    int i = heap->posicao[vertice];
    heap->chaves[i] = nova_chave; /* so diminui: conserta subindo */
    subir(heap, i);
}

int MinHeap_extract_min(MinHeap *heap) {
    if (heap->tamanho == 0) {
        return -1;
    }

    int vertice = heap->vertices[0];
    int ultimo = heap->tamanho - 1;
    heap->tamanho = ultimo;
    heap->posicao[vertice] = -1;

    if (ultimo > 0) {
        heap->vertices[0] = heap->vertices[ultimo];
        heap->chaves[0] = heap->chaves[ultimo];
        heap->posicao[heap->vertices[0]] = 0;
        descer(heap, 0);
    }
    return vertice;
}

bool MinHeap_isEmpty(MinHeap *heap) {
    return heap->tamanho == 0;
}

void MinHeap_destroy(MinHeap *heap) {
    free(heap->vertices);
    free(heap->chaves);
    free(heap->posicao);
    heap->vertices = NULL;
    heap->chaves = NULL;
    heap->posicao = NULL;
    heap->tamanho = 0;
    heap->capacidade = 0;
}

#ifdef HEAP_MAIN
/* if __name__ == "__main__" do heap */
#include <stdio.h>

int main(void) {
    MinHeap heap1;
    MinHeap_init(&heap1, 8);

    MinHeap_insert(&heap1, 1, 5.0);
    MinHeap_insert(&heap1, 2, 3.0);
    MinHeap_insert(&heap1, 3, 7.0);

    printf("%s\n", MinHeap_contains(&heap1, 1) ? "True" : "False");
    MinHeap_decrease_key(&heap1, 1, 1.5); /* o 1 sobe para a raiz */

    while (!MinHeap_isEmpty(&heap1)) {
        printf("%d\n", MinHeap_extract_min(&heap1));
    }
    printf("%s\n", MinHeap_isEmpty(&heap1) ? "True" : "False");

    MinHeap_destroy(&heap1);
    return 0;
}
#endif
