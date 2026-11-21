#include <stdio.h>
#include <stdlib.h>

#include "floyd_warshall.h"
#include "graph_utils.h"

/*
A inicializacao usa o iterador de vizinhanca (arestas de SAIDA, com os
pesos), entao serve para as duas representacoes, direcionado ou nao, e
leva O(n + m) em vez de espelhar a matriz inteira.

O triplo laco e a otimizacao de memoria em acao: UMA matriz dist, com o k
mais externo. A passada k so usa valores da passada k-1 - e as linhas
dist[k][*] e as colunas dist[*][k] que ela reescreve sao exatamente as
que nao mudam na passada k (intermediario k nao melhora caminho que ja
passa por k), por isso atualizar no lugar da o mesmo resultado que
guardar as n matrizes D[k].

Otimizacoes de tempo (as distancias sao as mesmas do triplo laco classico):

  - MATRIZ CONTIGUA: as n linhas moram num unico bloco de n*n (dist[i] e
    pred[i] continuam sendo ponteiros para as linhas, entao quem usa o
    resultado nao muda). Uma alocacao em vez de 2n, e as linhas vizinhas
    ficam vizinhas na memoria.
  - SEM TESTE DE INF NO LACO INTERNO: INFINITO_PESO e o +infinito do IEEE
    754, e inf + x = inf para qualquer x finito, negativo inclusive - um
    "infinito menor" nao existe, entao a soma ja da a resposta certa e o
    laco interno fica sem desvio (o compilador o vetoriza). O teste de
    dist[i][k] == INF continua, fora do laco interno: ele pula a linha
    inteira quando k nao serve de intermediario para i.
  - PARADA ANTECIPADA: assim que algum dist[i][i] fica negativo (conferido
    ao fim de cada faixa de 64 valores de k), o ciclo negativo esta provado
    e o algoritmo para.
  - EM BLOCOS (tiling): o FW classico percorre a matriz inteira (n^2
    doubles) a cada k, e para n grande ela nao cabe no cache - o tempo vai
    para a memoria, nao para as contas. A versao em blocos B x B processa B
    valores de k de uma vez sobre blocos que cabem no cache, na ordem que
    respeita as dependencias: (1) o bloco da diagonal (k, k); (2) os blocos
    da mesma linha e da mesma coluna, que dependem dele; (3) todos os
    outros, que dependem dos da fase 2. A complexidade continua O(n^3); o
    acesso a memoria cai cerca de B vezes. Em empates (dois caminhos minimos
    de mesmo custo) a ordem diferente pode escolher outro predecessor -
    outro caminho, igualmente minimo. Com n <= B ha um bloco so e a ordem e
    exatamente a classica.

O MODO ECONOMICO (floyd_warshall_economico) guarda as distancias em float:
8 n^2 bytes em vez de 12 n^2 - com n = 25000, 5 GB em vez de 7,5 GB. E o
MESMO nucleo (floyd_warshall_nucleo.h, instanciado para double e para
float); o preco e a precisao, cerca de 7 algarismos significativos.
*/
#define FW_BLOCO 64

/* as duas instancias do nucleo (floyd_warshall_nucleo.h): double e float */
#define TIPO double
#define SUF _d
#include "floyd_warshall_nucleo.h"
#undef TIPO
#undef SUF
#define TIPO float
#define SUF _f
#include "floyd_warshall_nucleo.h"
#undef TIPO
#undef SUF

/* aloca uma matriz n x n contigua de `tamanho` bytes por celula, com o
   vetor de ponteiros para as linhas (linhas[i] = bloco + i*n) */
static void **matriz_contigua(int n, size_t tamanho) {
    size_t celulas = (size_t)n * (size_t)n;
    char *bloco = xmalloc((celulas > 0 ? celulas : 1) * tamanho);
    void **linhas = xmalloc((size_t)(n > 0 ? n : 1) * sizeof(void *));
    linhas[0] = bloco; /* tambem com n == 0: o free usa linhas[0] */
    for (int i = 0; i < n; i++) {
        linhas[i] = bloco + (size_t)i * (size_t)n * tamanho;
    }
    return linhas;
}

static void liberar_matriz(void **linhas) {
    if (linhas != NULL) {
        free(linhas[0]); /* o bloco contiguo com todas as linhas */
        free(linhas);
    }
}

FloydWarshallResult *floyd_warshall(Graph *g) {
    FloydWarshallResult *result = xmalloc(sizeof(FloydWarshallResult));
    result->n = g->n;
    result->dist = (double **)matriz_contigua(g->n, sizeof(double));
    result->dist32 = NULL;
    result->pred = (int **)matriz_contigua(g->n, sizeof(int));
    result->negative_cycle = fw_rodar_d(g, result->dist, result->pred);
    return result;
}

FloydWarshallResult *floyd_warshall_economico(Graph *g) {
    FloydWarshallResult *result = xmalloc(sizeof(FloydWarshallResult));
    result->n = g->n;
    result->dist = NULL;
    result->dist32 = (float **)matriz_contigua(g->n, sizeof(float));
    result->pred = (int **)matriz_contigua(g->n, sizeof(int));
    result->negative_cycle = fw_rodar_f(g, result->dist32, result->pred);
    return result;
}

double floyd_warshall_dist(FloydWarshallResult *result, int origem, int destino) {
    if (result->dist != NULL) {
        return result->dist[origem][destino];
    }
    float d = result->dist32[origem][destino];
    return (d == (float)INFINITO_PESO) ? INFINITO_PESO : (double)d;
}

int *floyd_warshall_path(FloydWarshallResult *result, int origem, int destino,
                         int *comprimento_out) {
    if (floyd_warshall_dist(result, origem, destino) == INFINITO_PESO) {
        *comprimento_out = 0;
        return NULL;
    }

    /* caminha os predecessores de destino tras ate origem e inverte. Um
       caminho minimo e simples: no maximo n vertices; passar disso so e
       possivel com a arvore corrompida por ciclo negativo (nao confiavel)
       - o limite devolve NULL em vez de rodar para sempre. */
    int comprimento = 0;
    for (int v = destino; v != -1; v = result->pred[origem][v]) {
        comprimento += 1;
        if (comprimento > result->n) {
            *comprimento_out = 0;
            return NULL;
        }
    }

    int *caminho = xmalloc((size_t)comprimento * sizeof(int));
    int i = comprimento - 1;
    for (int v = destino; v != -1; v = result->pred[origem][v]) {
        caminho[i] = v;
        i -= 1;
    }

    *comprimento_out = comprimento;
    return caminho;
}

void FloydWarshallResult_free(FloydWarshallResult *result) {
    if (result == NULL) {
        return;
    }
    liberar_matriz((void **)result->dist);
    liberar_matriz((void **)result->dist32);
    liberar_matriz((void **)result->pred);
    free(result);
}

#ifdef FLOYD_WARSHALL_MAIN
/* if __name__ == "__main__" do floyd_warshall: o digrafo com pesos
   negativos (sem ciclo negativo), com as distancias entre todos os pares
   e um caminho reconstruido; depois o exemplo da Parte 2 como digrafo,
   que TEM ciclo negativo (3 -> 4 -> 5 -> 3, custo -2.2). */
#include <stdio.h>

int main(void) {
    const char *arquivos[2] = {"graph_teste_direcionado_negativo.txt",
                               "graph_exemplo_direcionado.txt"};
    const char *avisos[2] = {"(sem ciclo negativo)",
                             "(com ciclo negativo: dist nao confiavel)"};

    for (int f = 0; f < 2; f++) {
        Graph *g = Graph_create(arquivos[f], true, true);
        FloydWarshallResult *r = Graph_floyd_warshall(g);

        printf("--- %s %s ---\n", arquivos[f], avisos[f]);
        printf("ciclo negativo: %s\n", r->negative_cycle ? "SIM" : "nao");

        for (int i = 0; i < g->n; i++) {
            for (int j = 0; j < g->n; j++) {
                if (i == j || r->dist[i][j] == INFINITO_PESO) {
                    continue;
                }
                printf("d(%d,%d) = %g\n", i + 1, j + 1, r->dist[i][j]);
            }
        }

        if (!r->negative_cycle) {
            int comprimento = 0;
            int *caminho = floyd_warshall_path(r, 0, g->n - 1, &comprimento);
            printf("caminho 1 -> %d: %d", g->n, caminho[0] + 1);
            for (int i = 1; i < comprimento; i++) {
                printf(" -> %d", caminho[i] + 1);
            }
            printf(" (custo %g)\n", r->dist[0][g->n - 1]);
            free(caminho);
        }
        printf("\n");

        FloydWarshallResult_free(r);
        Graph_destroy(g);
    }
    return 0;
}
#endif
