#include <stdio.h>
#include <stdlib.h>

#include "bellman_ford.h"
#include "graph_utils.h"

/*
Bellman-Ford com as duas otimizacoes discutidas em aula (ver bellman_ford.h):
um so vetor de estimativas M[v], atualizado no lugar (memoria O(n), nao
O(n^2) da tabela M[k][v]), e a parada cedo quando uma passada nao relaxa
nada. A passada percorre os vertices em ordem crescente e relaxa as arestas
de SAIDA pelo iterador de vizinhanca - o mesmo do Dijkstra - entao o
algoritmo e um so e serve para as duas representacoes, direcionado ou nao.
*/
BellmanFordResult *bellman_ford(Graph *g, int fonte) {
    int n = g->n;

    double *dist = xmalloc((size_t)n * sizeof(double));
    int *parents = xmalloc((size_t)n * sizeof(int));
    for (int v = 0; v < n; v++) {
        dist[v] = INFINITO_PESO;
        parents[v] = -1; /* None no Python */
    }
    dist[fonte] = 0.0;

    int passadas = 0;
    bool melhorou = true; /* entra na 1a passada */

    /* n-1 passadas bastam: um caminho minimo (sem ciclo negativo no meio)
       tem no maximo n-1 arestas. A condicao `melhorou` e a TERMINACAO
       PRECOCE: passada que nao relaxa nada encerra o algoritmo. */
    for (int i = 1; i <= n - 1 && melhorou; i++) {
        melhorou = false;
        passadas = i;

        for (int u = 0; u < n; u++) {
            if (dist[u] == INFINITO_PESO) {
                continue; /* nada parte de um vertice inalcancavel */
            }
            int v;
            double peso;
            NeighborIter it = neighbor_iter(g, u);
            while (neighbor_next(&it, &v, &peso)) {
                double alternativa = dist[u] + peso; /* M[v] no lugar */
                if (alternativa < dist[v]) {
                    dist[v] = alternativa;
                    parents[v] = u;
                    melhorou = true;
                }
            }
        }
    }

    /* Deteccao de ciclo negativo: uma passada a mais, so para flagar (os
       valores nao sao confiaveis quando ha ciclo, entao nao vale
       atualizar nada). Ela so relaxa algo se ainda da para melhorar com
       mais de n-1 arestas - o que exige um vertice repetido no caminho,
       isto e, um ciclo negativo alcancavel da fonte. Se o laco de cima
       parou cedo (melhorou == false), esta passada tambem nao relaxaria
       nada e pode ser pulada. */
    bool ciclo_negativo = false;
    if (melhorou) {
        for (int u = 0; u < n && !ciclo_negativo; u++) {
            if (dist[u] == INFINITO_PESO) {
                continue;
            }
            int v;
            double peso;
            NeighborIter it = neighbor_iter(g, u);
            while (neighbor_next(&it, &v, &peso)) {
                if (dist[u] + peso < dist[v]) {
                    ciclo_negativo = true;
                    break;
                }
            }
        }
    }

    BellmanFordResult *result = xmalloc(sizeof(BellmanFordResult));
    result->n = n;
    result->dist = dist;
    result->parents = parents;
    result->negative_cycle = ciclo_negativo;
    result->passadas = passadas;
    return result;
}

BellmanFordResult *bellman_ford_to(Graph *g, int alvo) {
    /*
    "De todos para um": a distancia de v ate o alvo no grafo e a distancia
    do alvo ate v no TRANSPOSTO. Roda-se o Bellman-Ford classico no
    transposto, e o pai de v no transposto (o vertice antes de v no
    caminho alvo->v) e exatamente o vertice DEPOIS de v no caminho v->alvo
    no grafo original - a arvore devolvida ja "aponta para" o alvo.

    Grafo nao direcionado: o transposto e o proprio grafo (as arestas ja
    existem nas duas direcoes), e d(v,alvo) = d(alvo,v) - roda direto.
    */
    if (!g->directed) {
        return bellman_ford(g, alvo);
    }

    Graph *transposto = Graph_reverse(g);
    BellmanFordResult *result = bellman_ford(transposto, alvo);
    Graph_destroy(transposto);
    return result;
}

int *bellman_ford_path(BellmanFordResult *result, int alvo, int *comprimento_out) {
    if (result->dist[alvo] == INFINITO_PESO) {
        *comprimento_out = 0;
        return NULL;
    }

    /* caminha pela arvore de pais (alvo -> fonte) e inverte. Um caminho
       minimo e simples: tem no maximo n vertices. Passar disso so e
       possivel com a arvore corrompida por ciclo negativo (nao confiavel)
       - o limite devolve NULL em vez de rodar para sempre. */
    int comprimento = 0;
    for (int v = alvo; v != -1; v = result->parents[v]) {
        comprimento += 1;
        if (comprimento > result->n) {
            *comprimento_out = 0;
            return NULL;
        }
    }

    int *caminho = xmalloc((size_t)comprimento * sizeof(int));
    int i = comprimento - 1;
    for (int v = alvo; v != -1; v = result->parents[v]) {
        caminho[i] = v;
        i -= 1;
    }

    *comprimento_out = comprimento;
    return caminho;
}

int *bellman_ford_path_to(BellmanFordResult *result, int origem, int *comprimento_out) {
    if (result->dist[origem] == INFINITO_PESO) {
        *comprimento_out = 0;
        return NULL;
    }

    /* aqui a arvore aponta para o alvo: caminha-se de origem PARA a frente
       (origem -> proximo -> ... -> alvo, que tem parents == -1), sem
       inverter nada. Mesmo guarda-feio do que o de cima: mais de n passos
       = arvore corrompida por ciclo negativo. */
    int comprimento = 0;
    for (int v = origem; v != -1; v = result->parents[v]) {
        comprimento += 1;
        if (comprimento > result->n) {
            *comprimento_out = 0;
            return NULL;
        }
    }

    int *caminho = xmalloc((size_t)comprimento * sizeof(int));
    int i = 0;
    for (int v = origem; v != -1; v = result->parents[v]) {
        caminho[i] = v;
        i += 1;
    }

    *comprimento_out = comprimento;
    return caminho;
}

void BellmanFordResult_free(BellmanFordResult *result) {
    if (result == NULL) {
        return;
    }
    free(result->dist);
    free(result->parents);
    free(result);
}

#ifdef BELLMAN_FORD_MAIN
/* if __name__ == "__main__" do bellman_ford: o exemplo com peso negativo
   da Parte 2 (Figura 1), agora como DIGRAFO - o Dijkstra recusa, o
   Bellman-Ford resolve... ate encontrar o ciclo negativo 3->4->5->3. */
#include <stdio.h>

static void imprimir(const char *titulo, const char *formato, BellmanFordResult *r, int n) {
    printf("--- %s ---\n", titulo);
    if (r->negative_cycle) {
        printf("ciclo negativo alcancavel (%d passadas nao bastaram)\n", r->passadas);
    } else {
        for (int v = 0; v < n; v++) {
            printf(formato, v + 1, r->dist[v]);
        }
    }
    BellmanFordResult_free(r);
}

int main(void) {
    Graph *g = Graph_create("graph_exemplo_direcionado.txt", true, true);

    imprimir("de todos para o vertice 5", "d(%d,5) = %g\n",
             Graph_bellman_ford_to(g, 5), g->n);
    printf("\n");
    imprimir("do vertice 1 para todos", "d(1,%d) = %g\n",
             Graph_bellman_ford(g, 1), g->n);

    Graph *p = Graph_create("graph_teste_direcionado_negativo.txt", true, true);
    printf("\n(um digrafo com pesos negativos, SEM ciclo negativo)\n");
    imprimir("de todos para o vertice 5", "d(%d,5) = %g\n",
             Graph_bellman_ford_to(p, 5), p->n);
    printf("\n");

    BellmanFordResult *r = Graph_bellman_ford(p, 1);
    int comprimento = 0;
    int *caminho = bellman_ford_path(r, 4, &comprimento);
    printf("caminho 1 -> 5: 1");
    for (int i = 1; i < comprimento; i++) {
        printf(" -> %d", caminho[i] + 1);
    }
    printf(" (custo %g)\n", r->dist[4]);
    free(caminho);
    BellmanFordResult_free(r);

    Graph_destroy(g);
    Graph_destroy(p);
    return 0;
}
#endif
