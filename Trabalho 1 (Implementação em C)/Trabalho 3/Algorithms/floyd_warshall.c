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

Os INF sao testados ANTES de somar: infinito + peso negativo nao pode
virar um "infinito menor" e roubar o lugar de um caminho que existe.
*/
FloydWarshallResult *floyd_warshall(Graph *g) {
    int n = g->n;

    double **dist = xmalloc((size_t)n * sizeof(double *));
    int **pred = xmalloc((size_t)n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        dist[i] = xmalloc((size_t)n * sizeof(double));
        pred[i] = xmalloc((size_t)n * sizeof(int));
        for (int j = 0; j < n; j++) {
            dist[i][j] = INFINITO_PESO;
            pred[i][j] = -1; /* sem aresta direta */
        }
        dist[i][i] = 0.0; /* caminho vazio de i a i */
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

    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dist[i][k] == INFINITO_PESO) {
                continue; /* k nao serve de intermediario para i */
            }
            double dik = dist[i][k];
            double *dist_k = dist[k]; /* so alias, para indexar uma vez */
            int *pred_k = pred[k];
            for (int j = 0; j < n; j++) {
                if (dist_k[j] == INFINITO_PESO) {
                    continue; /* j nao e alcancavel a partir de k */
                }
                double alternativa = dik + dist_k[j];
                if (alternativa < dist[i][j]) {
                    dist[i][j] = alternativa; /* no lugar: memoria O(n^2) */
                    /* o ultimo passo do caminho i->k->j e o mesmo do
                       caminho k->j: herda o predecessor */
                    pred[i][j] = pred_k[j];
                }
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
    for (int i = 0; i < result->n; i++) {
        free(result->dist[i]);
        free(result->pred[i]);
    }
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
