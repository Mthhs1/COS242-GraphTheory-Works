/*
O preco da "saida facil" (dica do professor no site da disciplina): em vez de
achar o vertice no heap para diminuir a chave (decrease-key, que o nosso
MinHeap faz em O(1) + O(log n) com posicao[]), reinserir o vertice a cada
melhora e descartar as copias velhas quando sairem do heap.

Para os mesmos k vertices iniciais da questao 2 (mesma semente), roda o
Dijkstra com DIJKSTRA_HEAP e com DIJKSTRA_HEAP_REINSERCAO e compara:
  - tempo medio (so o algoritmo, como na questao 2);
  - tamanho maximo do heap (entradas) e a memoria que isso representa;
  - extracoes descartadas (copias velhas) na reinsercao;
e confere que as distancias sao identicas nas duas.

uso: ./bin/p2_saida_facil <grafo.txt> [k]
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "graph.h"
#include "graph_utils.h"
#include "dijkstra.h"

static double tempo_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

/* o mesmo xorshift64* com a mesma semente da questao 2 */
static unsigned long estado = 20260101UL;

static int proximo_aleatorio(int n) {
    estado ^= estado << 13;
    estado ^= estado >> 7;
    estado ^= estado << 17;
    return (int)((estado * 0x2545F4914F6CDD1DUL >> 33) % (unsigned long)n) + 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "uso: %s <grafo.txt> [k]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int k = (argc >= 3) ? atoi(argv[2]) : 20;

    Graph *g = Graph_create(argv[1], true);
    if (g->negative_weight) {
        fprintf(stderr, "grafo com peso negativo: Dijkstra nao se aplica\n");
        Graph_destroy(g);
        return EXIT_FAILURE;
    }
    if (k > g->n) {
        k = g->n;
    }

    bool *usado = xcalloc((size_t)g->n, sizeof(bool));
    int *inicio = xmalloc((size_t)k * sizeof(int));
    for (int escolhidos = 0; escolhidos < k;) {
        int v = proximo_aleatorio(g->n);
        if (!usado[v - 1]) {
            usado[v - 1] = true;
            inicio[escolhidos++] = v - 1;
        }
    }
    free(usado);

    double t_heap = 0.0, t_re = 0.0;
    double pico_heap = 0.0, pico_re = 0.0, vazias_re = 0.0;
    long pico_re_max = 0;
    int divergencias = 0;

    for (int i = 0; i < k; i++) {
        double t0 = tempo_ms();
        DijkstraResult *rh = dijkstra(g, inicio[i], DIJKSTRA_HEAP);
        double t1 = tempo_ms();
        DijkstraResult *rr = dijkstra(g, inicio[i], DIJKSTRA_HEAP_REINSERCAO);
        double t2 = tempo_ms();

        t_heap += t1 - t0;
        t_re += t2 - t1;
        pico_heap += (double)rh->heap_pico;
        pico_re += (double)rr->heap_pico;
        vazias_re += (double)rr->extracoes_vazias;
        if (rr->heap_pico > pico_re_max) {
            pico_re_max = rr->heap_pico;
        }
        if (memcmp(rh->dist, rr->dist, (size_t)g->n * sizeof(double)) != 0) {
            divergencias += 1;
        }

        DijkstraResult_free(rh);
        DijkstraResult_free(rr);
    }
    free(inicio);

    /* bytes por entrada: MinHeap = vertice (4) + chave (8) + posicao[] (4);
       reinsercao = struct {double chave; int vertice} (16, com alinhamento) */
    printf("grafo: %s | n = %d | %d dijkstras por modo (mesmos inicios da questao 2)\n",
           argv[1], g->n, k);
    printf("decrease-key  tempo medio: %.4f ms | pico medio do heap: %.0f entradas (%.1f MB alocados: 16 bytes x n)\n",
           t_heap / k, pico_heap / k, 16.0 * g->n / 1048576.0);
    printf("reinsercao    tempo medio: %.4f ms | pico medio do heap: %.0f entradas (maximo %ld; %.1f MB no pico medio)\n",
           t_re / k, pico_re / k, pico_re_max, 16.0 * pico_re / k / 1048576.0);
    printf("reinsercao    extracoes descartadas (copias velhas) por execucao: %.0f\n", vazias_re / k);
    printf("distancias identicas nas duas: %s\n", divergencias == 0 ? "sim" : "NAO");

    Graph_destroy(g);
    return divergencias == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
