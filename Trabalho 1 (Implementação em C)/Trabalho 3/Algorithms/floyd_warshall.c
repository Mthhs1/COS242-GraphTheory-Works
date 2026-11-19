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
*/
#define FW_BLOCO 64

/* o FW classico restrito a: k em [kb, kb+nk), i em [ib, ib+ni), j em [jb, jb+nj) */
static void fw_bloco(double **dist, int **pred, int kb, int nk,
                     int ib, int ni, int jb, int nj) {
    for (int k = kb; k < kb + nk; k++) {
        const double *dist_k = dist[k];
        const int *pred_k = pred[k];
        for (int i = ib; i < ib + ni; i++) {
            double dik = dist[i][k];
            if (dik == INFINITO_PESO) {
                continue; /* k nao serve de intermediario para i */
            }
            double *dist_i = dist[i];
            int *pred_i = pred[i];
            for (int j = jb; j < jb + nj; j++) {
                double alternativa = dik + dist_k[j]; /* inf + x = inf */
                if (alternativa < dist_i[j]) {
                    dist_i[j] = alternativa; /* no lugar: memoria O(n^2) */
                    /* o ultimo passo do caminho i->k->j e o mesmo do
                       caminho k->j: herda o predecessor */
                    pred_i[j] = pred_k[j];
                }
            }
        }
    }
}

FloydWarshallResult *floyd_warshall(Graph *g) {
    int n = g->n;

    double **dist = xmalloc((size_t)(n > 0 ? n : 1) * sizeof(double *));
    int **pred = xmalloc((size_t)(n > 0 ? n : 1) * sizeof(int *));
    double *bloco_dist = xmalloc(((size_t)n * (size_t)n > 0 ? (size_t)n * (size_t)n : 1) * sizeof(double));
    int *bloco_pred = xmalloc(((size_t)n * (size_t)n > 0 ? (size_t)n * (size_t)n : 1) * sizeof(int));
    for (int i = 0; i < n; i++) {
        dist[i] = bloco_dist + (size_t)i * (size_t)n;
        pred[i] = bloco_pred + (size_t)i * (size_t)n;
        for (int j = 0; j < n; j++) {
            dist[i][j] = INFINITO_PESO;
            pred[i][j] = -1; /* sem aresta direta */
        }
        dist[i][i] = 0.0; /* caminho vazio de i a i */
    }
    if (n == 0) {
        dist[0] = bloco_dist; /* para o free do resultado */
        pred[0] = bloco_pred;
    }

    /* as arestas do arquivo: dist[i][j] = peso(i, j), pred[i][j] = i */
    for (int i = 0; i < n; i++) {
        int j;
        double peso;
        NeighborIter it = neighbor_iter(g, i);
        while (neighbor_next(&it, &j, &peso)) {
            if (peso < dist[i][j]) { /* entre duplicatas vale a menor: a
                                        leitura ja fundiu, isto e so defesa */
                dist[i][j] = peso;
                pred[i][j] = i;
            }
        }
    }

    /* as tres fases para cada faixa de k [kb, kb+nk) */
    for (int kb = 0; kb < n; kb += FW_BLOCO) {
        int nk = (n - kb < FW_BLOCO) ? n - kb : FW_BLOCO;

        /* (1) o bloco da diagonal */
        fw_bloco(dist, pred, kb, nk, kb, nk, kb, nk);

        /* (2) a linha e a coluna de blocos da faixa k */
        for (int b = 0; b < n; b += FW_BLOCO) {
            if (b == kb) {
                continue;
            }
            int nb = (n - b < FW_BLOCO) ? n - b : FW_BLOCO;
            fw_bloco(dist, pred, kb, nk, kb, nk, b, nb); /* linha  */
            fw_bloco(dist, pred, kb, nk, b, nb, kb, nk); /* coluna */
        }

        /* (3) todo o resto */
        for (int ib = 0; ib < n; ib += FW_BLOCO) {
            if (ib == kb) {
                continue;
            }
            int ni = (n - ib < FW_BLOCO) ? n - ib : FW_BLOCO;
            for (int jb = 0; jb < n; jb += FW_BLOCO) {
                if (jb == kb) {
                    continue;
                }
                int nj = (n - jb < FW_BLOCO) ? n - jb : FW_BLOCO;
                fw_bloco(dist, pred, kb, nk, ib, ni, jb, nj);
            }
        }
    }

    /* ciclo negativo: o menor passeio fechado por i custa menos que 0.
       Um passeio fechado negativo sempre passa por um ciclo negativo - e
       qualquer ciclo negativo do grafo aparece no vertice dele. */
    bool ciclo_negativo = false;
    for (int i = 0; i < n; i++) {
        if (dist[i][i] < 0.0) {
            ciclo_negativo = true;
            break;
        }
    }

    FloydWarshallResult *result = xmalloc(sizeof(FloydWarshallResult));
    result->n = n;
    result->dist = dist;
    result->pred = pred;
    result->negative_cycle = ciclo_negativo;
    return result;
}

int *floyd_warshall_path(FloydWarshallResult *result, int origem, int destino,
                         int *comprimento_out) {
    if (result->dist[origem][destino] == INFINITO_PESO) {
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
    free(result->dist[0]); /* o bloco contiguo com todas as linhas */
    free(result->pred[0]);
    free(result->dist);
    free(result->pred);
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
