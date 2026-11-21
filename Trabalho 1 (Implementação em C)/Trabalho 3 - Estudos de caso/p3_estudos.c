/*
Estudos de caso da Parte 3 (secao 3 do enunciado do trabalho-P3), para UM
grafo direcionado com pesos:

  Q1  Bellman-Ford: distancia dos vertices 10, 20 e 30 PARA o vertice 100
      (o "de todos para um" - bellman_ford_to, sobre o transposto);
  Q2  Floyd-Warshall: distancias entre os pares de {10, 20, 30, 40, 50};
  Q3  tempo medio de cada algoritmo (o BF da Q1 e o FW), em R rodadas;
  Q4  todos os pares pelo Bellman-Ford (uma execucao por fonte), tempo medio
      em R rodadas, contra o Floyd-Warshall. A mais, o Johnson (potencial
      por Bellman-Ford + um Dijkstra com heap por fonte), a otimizacao que
      baixa a complexidade de O(n^3) / O(n^2 m) para O(nm log n).

O cronometro cobre so o algoritmo: a leitura do arquivo e a escrita da
saida ficam fora (como pede o enunciado). Os resultados de cada execucao de
todos os pares sao descartados linha a linha (calcular as n^2 distancias e
o trabalho medido); a conferencia entre os metodos e feita fora do
cronometro, numa execucao separada.

uso: ./bin/p3_estudos <grafo.txt> [rodadas=10] [questoes=1234]
     ex.: ./bin/p3_estudos ../Grafos/grafo_1.txt 10 1234

Variaveis de ambiente (para grafos grandes demais):
     FW_MAX_N=n          nao roda o Floyd-Warshall se n > FW_MAX_N
                         (padrao: o exato se 12 n^2 bytes cabem em 80% da
                         memoria livre; senao o economico, se 8 n^2 cabem)
     FW_RODADAS=r        rodadas do FW na Q3 (padrao: as mesmas R; a do FW
                         da Q2 conta como a primeira)
     TODOS_PARES_RODADAS=r  rodadas da Q4 (padrao: as mesmas R)
     TODOS_PARES_AMOSTRA=k  na Q4, se > 0, cronometra so k fontes e
                         extrapola para n (marcado como ESTIMADO na saida)
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "graph.h"
#include "graph_utils.h"
#include "dijkstra.h"
#include "bellman_ford.h"
#include "floyd_warshall.h"
#include "johnson.h"

#define N_Q1 3
static const int Q1_ORIGENS[N_Q1] = {10, 20, 30};
static const int Q1_ALVO = 100;
#define N_Q2 5
static const int Q2_VERTICES[N_Q2] = {10, 20, 30, 40, 50};

static double agora_s(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static long env_long(const char *nome, long padrao) {
    const char *v = getenv(nome);
    return (v != NULL && *v != '\0') ? atol(v) : padrao;
}

/* memoria livre (MemAvailable do /proc/meminfo), em bytes; 0 se nao sabe */
static double memoria_livre(void) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (f == NULL) {
        return 0.0;
    }
    char linha[256];
    double kb = 0.0;
    while (fgets(linha, sizeof linha, f) != NULL) {
        if (sscanf(linha, "MemAvailable: %lf kB", &kb) == 1) {
            break;
        }
    }
    fclose(f);
    return kb * 1024.0;
}

static void imprimir_dist(double d) {
    if (d == INFINITO_PESO) {
        printf("inf");
    } else {
        printf("%.10g", d);
    }
}

static bool rotulo_valido(Graph *g, int r) {
    return r >= 1 && r <= g->n;
}

/* ------------------------------------------------------------------ */

static void questao1(Graph *g) {
    printf("\n== Q1: Bellman-Ford, distancia de 10, 20 e 30 para o vertice %d ==\n", Q1_ALVO);
    if (!rotulo_valido(g, Q1_ALVO)) {
        printf("vertice %d nao existe (n = %d)\n", Q1_ALVO, g->n);
        return;
    }
    BellmanFordResult *r = bellman_ford_to(g, Q1_ALVO - 1);
    printf("ciclo negativo que chega ao %d: %s | passadas: %d\n", Q1_ALVO,
           r->negative_cycle ? "SIM (distancias nao confiaveis)" : "nao", r->passadas);
    for (int i = 0; i < N_Q1; i++) {
        int o = Q1_ORIGENS[i];
        if (!rotulo_valido(g, o)) {
            printf("q1 %d inexistente\n", o);
            continue;
        }
        printf("q1 %d ", o);
        imprimir_dist(r->dist[o - 1]);
        int comprimento = 0;
        int *caminho = r->negative_cycle ? NULL : bellman_ford_path_to(r, o - 1, &comprimento);
        printf(" | %d arestas | caminho:", comprimento > 0 ? comprimento - 1 : 0);
        for (int k = 0; k < comprimento; k++) {
            printf("%s%d", k ? " -> " : " ", caminho[k] + 1);
        }
        printf("\n");
        free(caminho);
    }
    BellmanFordResult_free(r);
}

/* Qual Floyd-Warshall cabe na memoria livre: 1 = exato (double + int,
   12 n^2 bytes), 2 = economico (float + int, 8 n^2), 0 = nenhum. */
#define FW_NENHUM 0
#define FW_EXATO 1
#define FW_ECONOMICO 2
static int fw_modo(Graph *g) {
    double n2 = (double)g->n * (double)g->n;
    long max_n = env_long("FW_MAX_N", 0);
    if (max_n > 0 && g->n > max_n) {
        return FW_NENHUM;
    }
    double livre = memoria_livre();
    if (livre <= 0.0 || 12.0 * n2 <= 0.8 * livre) {
        return FW_EXATO;
    }
    if (8.0 * n2 <= 0.8 * livre) {
        return FW_ECONOMICO;
    }
    return FW_NENHUM;
}

static FloydWarshallResult *fw_rodar(Graph *g, int modo) {
    return modo == FW_ECONOMICO ? floyd_warshall_economico(g) : floyd_warshall(g);
}

/* o tempo do FW da Q2 vale como 1a rodada da Q3 (com n = 25000 cada FW leva
   dezenas de minutos: nao se repete a conta so para medir) */
static double fw_tempo_q2 = -1.0;

static void questao2(Graph *g) {
    printf("\n== Q2: Floyd-Warshall, distancias entre {10, 20, 30, 40, 50} ==\n");
    int modo = fw_modo(g);
    if (modo == FW_NENHUM) {
        printf("floyd-warshall inviavel: precisaria de %.1f GB (8 n^2 bytes no modo economico)\n",
               8.0 * (double)g->n * (double)g->n / 1073741824.0);
        return;
    }
    printf("modo: %s\n", modo == FW_EXATO ? "exato (double, 12 n^2 bytes)"
                                          : "economico (float, 8 n^2 bytes)");
    double t0 = agora_s();
    FloydWarshallResult *r = fw_rodar(g, modo);
    fw_tempo_q2 = agora_s() - t0;
    printf("tempo deste floyd_warshall %.3f s\n", fw_tempo_q2);
    printf("ciclo negativo no grafo: %s\n", r->negative_cycle ? "SIM (distancias nao confiaveis)" : "nao");
    for (int a = 0; a < N_Q2; a++) {
        for (int b = 0; b < N_Q2; b++) {
            int i = Q2_VERTICES[a], j = Q2_VERTICES[b];
            if (!rotulo_valido(g, i) || !rotulo_valido(g, j)) {
                continue;
            }
            printf("q2 %d %d ", i, j);
            imprimir_dist(floyd_warshall_dist(r, i - 1, j - 1));
            printf("\n");
        }
    }

    /* conferencia, fora do cronometro: as linhas do FW das fontes da Q2 contra
       o Bellman-Ford de cada uma (o FW nao e recalculado em outro lugar) */
    if (!r->negative_cycle) {
        double maior = 0.0;
        long divergentes = 0;
        for (int a = 0; a < N_Q2; a++) {
            int s = Q2_VERTICES[a] - 1;
            if (s >= g->n) {
                continue;
            }
            BellmanFordResult *bf = bellman_ford(g, s);
            for (int v = 0; v < g->n; v++) {
                double x = floyd_warshall_dist(r, s, v), y = bf->dist[v];
                if ((x == INFINITO_PESO) != (y == INFINITO_PESO)) {
                    divergentes += 1;
                } else if (x != INFINITO_PESO) {
                    maior = fmax(maior, fabs(x - y));
                }
            }
            BellmanFordResult_free(bf);
        }
        printf("conferencia FW x BF (linhas de 10..50 inteiras): maior diferenca %.3g, "
               "alcancabilidade divergente em %ld pares\n", maior, divergentes);
    }
    FloydWarshallResult_free(r);
}

/* ------------------------------------------------------------------ */

static void questao3(Graph *g, int rodadas) {
    printf("\n== Q3: tempo medio de cada algoritmo (%d rodadas, s) ==\n", rodadas);
    if (rotulo_valido(g, Q1_ALVO)) {
        double total = 0.0, minimo = 1e300, maximo = 0.0;
        for (int k = 0; k < rodadas; k++) {
            double t0 = agora_s();
            BellmanFordResult *r = bellman_ford_to(g, Q1_ALVO - 1);
            double dt = agora_s() - t0;
            BellmanFordResult_free(r);
            total += dt;
            minimo = dt < minimo ? dt : minimo;
            maximo = dt > maximo ? dt : maximo;
        }
        printf("tempo bellman_ford_to %.6f min %.6f max %.6f\n", total / rodadas, minimo, maximo);
    }
    int modo = fw_modo(g);
    if (modo != FW_NENHUM) {
        int r_fw = (int)env_long("FW_RODADAS", rodadas);
        double total = 0.0, minimo = 1e300, maximo = 0.0;
        int feitas = 0;
        if (fw_tempo_q2 >= 0.0) { /* a rodada da Q2 conta */
            total = minimo = maximo = fw_tempo_q2;
            feitas = 1;
        }
        for (; feitas < r_fw; feitas++) {
            double t0 = agora_s();
            FloydWarshallResult *r = fw_rodar(g, modo);
            double dt = agora_s() - t0;
            FloydWarshallResult_free(r);
            total += dt;
            minimo = dt < minimo ? dt : minimo;
            maximo = dt > maximo ? dt : maximo;
        }
        printf("tempo floyd_warshall %.6f min %.6f max %.6f rodadas %d modo %s\n", total / feitas,
               minimo, maximo, feitas, modo == FW_EXATO ? "exato" : "economico");
    } else {
        printf("tempo floyd_warshall inviavel\n");
    }
}

/* ------------------------------------------------------------------ */
/* Q4: todos os pares                                                  */
/* ------------------------------------------------------------------ */

/* tempo de uma execucao de todos os pares por n Bellman-Fords; com amostra
   k > 0, mede k fontes espalhadas e extrapola para n */
static double todos_pares_bf(Graph *g, int amostra) {
    int fontes = (amostra > 0 && amostra < g->n) ? amostra : g->n;
    double t0 = agora_s();
    for (int i = 0; i < fontes; i++) {
        int s = (fontes == g->n) ? i : (int)((long)i * g->n / fontes);
        BellmanFordResult *r = bellman_ford(g, s);
        BellmanFordResult_free(r);
    }
    double dt = agora_s() - t0;
    return dt * (double)g->n / (double)fontes;
}

static double todos_pares_johnson(Graph *g, int amostra, bool *ciclo) {
    int fontes = (amostra > 0 && amostra < g->n) ? amostra : g->n;
    double t0 = agora_s();
    Johnson *j = johnson_preparar(g);
    double t_pot = agora_s() - t0;
    *ciclo = j->negative_cycle;
    double t1 = agora_s();
    if (!j->negative_cycle) {
        for (int i = 0; i < fontes; i++) {
            int s = (fontes == g->n) ? i : (int)((long)i * g->n / fontes);
            DijkstraResult *r = johnson_fonte(j, g, s);
            DijkstraResult_free(r);
        }
    }
    double t_dij = agora_s() - t1;
    Johnson_free(j);
    return t_pot + t_dij * (double)g->n / (double)fontes;
}

/* fora do cronometro: as tres formas de todos os pares dao as mesmas
   distancias? Com o FW so em grafo pequeno (n <= 5000, exato): nos grandes
   ele ja foi conferido contra o BF na Q2 e recalcula-lo custaria horas. */
static void conferir_todos_pares(Graph *g, int amostra) {
    int fontes = (amostra > 0 && amostra < g->n) ? amostra : g->n;
    FloydWarshallResult *fw = (g->n <= 5000 && fw_modo(g) == FW_EXATO) ? floyd_warshall(g) : NULL;
    Johnson *j = johnson_preparar(g);
    if (j->negative_cycle) {
        printf("conferencia: ciclo negativo no grafo (johnson%s) - distancias nao definidas\n",
               fw != NULL ? (fw->negative_cycle ? " e FW concordam" : " DISCORDA do FW") : "");
        Johnson_free(j);
        FloydWarshallResult_free(fw);
        return;
    }
    double max_dif = 0.0;
    long pares = 0, inf_diferentes = 0;
    for (int i = 0; i < fontes; i++) {
        int s = (fontes == g->n) ? i : (int)((long)i * g->n / fontes);
        BellmanFordResult *bf = bellman_ford(g, s);
        DijkstraResult *jo = johnson_fonte(j, g, s);
        for (int v = 0; v < g->n; v++) {
            double a = bf->dist[v], b = jo->dist[v];
            double c = fw != NULL ? floyd_warshall_dist(fw, s, v) : a;
            bool ia = a == INFINITO_PESO, ib = b == INFINITO_PESO, ic = c == INFINITO_PESO;
            if (ia != ib || ia != ic) {
                inf_diferentes += 1;
                continue;
            }
            if (!ia) {
                double d = fmax(fabs(a - b), fabs(a - c));
                max_dif = d > max_dif ? d : max_dif;
                pares += 1;
            }
        }
        BellmanFordResult_free(bf);
        DijkstraResult_free(jo);
    }
    printf("conferencia: %d fontes, %ld pares alcancaveis, maior diferenca %.3g, "
           "alcancabilidade divergente em %ld pares (%s)\n",
           fontes, pares, max_dif, inf_diferentes,
           fw != NULL ? "BF x Johnson x FW" : "BF x Johnson; FW inviavel");
    Johnson_free(j);
    FloydWarshallResult_free(fw);
}

static void questao4(Graph *g, int rodadas) {
    int r4 = (int)env_long("TODOS_PARES_RODADAS", rodadas);
    int amostra = (int)env_long("TODOS_PARES_AMOSTRA", 0);
    bool estimado = amostra > 0 && amostra < g->n;
    printf("\n== Q4: todos os pares (%d rodadas, s)%s ==\n", r4,
           estimado ? " - ESTIMADO por amostra de fontes" : "");
    if (estimado) {
        printf("amostra: %d fontes de %d, tempo extrapolado para n\n", amostra, g->n);
    }

    double total = 0.0;
    for (int k = 0; k < r4; k++) {
        total += todos_pares_bf(g, amostra);
    }
    printf("tempo todos_pares_bellman_ford %.6f\n", total / r4);

    total = 0.0;
    bool ciclo = false;
    for (int k = 0; k < r4; k++) {
        total += todos_pares_johnson(g, amostra, &ciclo);
    }
    printf("tempo todos_pares_johnson %.6f%s\n", total / r4,
           ciclo ? " (parou no ciclo negativo)" : "");

    conferir_todos_pares(g, amostra);
}

/* ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "uso: %s <grafo.txt> [rodadas=10] [questoes=1234]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int rodadas = argc >= 3 ? atoi(argv[2]) : 10;
    const char *questoes = argc >= 4 ? argv[3] : "1234";
    if (rodadas <= 0) {
        rodadas = 10;
    }

    double t0 = agora_s();
    Graph *g = Graph_create(argv[1], true, true); /* lista, DIRECIONADO */
    double t_leitura = agora_s() - t0;

    long m = 0;
    for (int v = 0; v < g->n; v++) {
        int w;
        double peso;
        NeighborIter it = neighbor_iter(g, v);
        while (neighbor_next(&it, &w, &peso)) {
            m += 1;
        }
    }
    printf("grafo: %s\n", argv[1]);
    printf("n %d m %ld pesos_negativos %s leitura %.3f s\n", g->n, m,
           g->negative_weight ? "sim" : "nao", t_leitura);

    if (strchr(questoes, '1')) questao1(g);
    if (strchr(questoes, '2')) questao2(g);
    if (strchr(questoes, '3')) questao3(g, rodadas);
    if (strchr(questoes, '4')) questao4(g, rodadas);

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
