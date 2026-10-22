/*
Questao 2 da Parte 2 (estudos de caso): tempo medio do Dijkstra com vetor
x com heap, a partir de k vertices iniciais escolhidos aleatoriamente
(k padrao 100, como sugere o enunciado).

A escolha e pseudo-aleatoria com semente FIXA (xorshift64*): reproduzivel
entre execucoes e entre maquinas, e os k vertices sao distintos.

Como nas questoes de tempo da Parte 1, o cronometro cobre apenas o
algoritmo: a leitura do grafo fica fora da medida.

uso: ./bin/p2_questao2 <grafo.txt> <matriz|lista> [k] [vetor|heap|ambos]
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

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <matriz|lista> [k] [vetor|heap|ambos]\n",
            programa);
    exit(EXIT_FAILURE);
}

static double tempo_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

/* xorshift64* com semente fixa: mesma sequencia em toda execucao */
static unsigned long estado = 20260101UL;

static int proximo_aleatorio(int n) {
    estado ^= estado << 13;
    estado ^= estado >> 7;
    estado ^= estado << 17;
    return (int)((estado * 0x2545F4914F6CDD1DUL >> 33) % (unsigned long)n) + 1;
}

typedef struct {
    double soma;
    double minimo;
    double maximo;
} Estatisticas;

static void acumular(Estatisticas *e, double duracao) {
    e->soma += duracao;
    if (e->minimo < 0 || duracao < e->minimo) {
        e->minimo = duracao;
    }
    if (duracao > e->maximo) {
        e->maximo = duracao;
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        uso(argv[0]);
    }

    bool adj_list;
    if (strcmp(argv[2], "lista") == 0) {
        adj_list = true;
    } else if (strcmp(argv[2], "matriz") == 0) {
        adj_list = false;
    } else {
        fprintf(stderr, "representacao invalida: %s (use matriz ou lista)\n", argv[2]);
        return EXIT_FAILURE;
    }

    int k = 100;
    if (argc >= 4) {
        k = atoi(argv[3]);
        if (k <= 0) {
            fprintf(stderr, "k invalido: %s\n", argv[3]);
            return EXIT_FAILURE;
        }
    }

    const char *modo_arg = (argc >= 5) ? argv[4] : "ambos";
    bool rodar_vetor = strcmp(modo_arg, "vetor") == 0 || strcmp(modo_arg, "ambos") == 0;
    bool rodar_heap = strcmp(modo_arg, "heap") == 0 || strcmp(modo_arg, "ambos") == 0;
    if (!rodar_vetor && !rodar_heap) {
        fprintf(stderr, "modo invalido: %s (use vetor, heap ou ambos)\n", modo_arg);
        return EXIT_FAILURE;
    }

    Graph *g = Graph_create(argv[1], adj_list);

    if (g->n < k) {
        k = g->n; /* grafos pequenos: usa todos os vertices */
    }
    if (k <= 0) {
        fprintf(stderr, "grafo sem vertices\n");
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    printf("grafo: %s | representacao: %s de adjacencia | %d dijkstras por modo\n",
           argv[1], argv[2], k);

    if (g->negative_weight) {
        fprintf(stderr,
                "o grafo tem aresta(s) com peso negativo: a biblioteca ainda "
                "nao implementa caminhos minimos com pesos negativos\n");
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    /* k vertices distintos, aleatorios (semente fixa), rotulos 1-based */
    bool *usado = xcalloc((size_t)g->n, sizeof(bool));
    int *inicio = xmalloc((size_t)k * sizeof(int));
    int escolhidos = 0;
    while (escolhidos < k) {
        int v = proximo_aleatorio(g->n);
        if (!usado[v - 1]) {
            usado[v - 1] = true;
            inicio[escolhidos] = v - 1;
            escolhidos += 1;
        }
    }
    free(usado);

    Estatisticas vetor = {0.0, -1.0, -1.0};
    Estatisticas heap = {0.0, -1.0, -1.0};

    for (int i = 0; i < k; i++) {
        if (rodar_vetor) {
            double t0 = tempo_ms();
            DijkstraResult *r = dijkstra(g, inicio[i], DIJKSTRA_VETOR);
            double t1 = tempo_ms();
            DijkstraResult_free(r);
            acumular(&vetor, t1 - t0);
        }
        if (rodar_heap) {
            double t0 = tempo_ms();
            DijkstraResult *r = dijkstra(g, inicio[i], DIJKSTRA_HEAP);
            double t1 = tempo_ms();
            DijkstraResult_free(r);
            acumular(&heap, t1 - t0);
        }
    }
    free(inicio);

    if (rodar_vetor) {
        printf("vetor media: %.4f ms (min %.4f, max %.4f)\n",
               vetor.soma / k, vetor.minimo, vetor.maximo);
    }
    if (rodar_heap) {
        printf("heap   media: %.4f ms (min %.4f, max %.4f)\n",
               heap.soma / k, heap.minimo, heap.maximo);
    }

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
