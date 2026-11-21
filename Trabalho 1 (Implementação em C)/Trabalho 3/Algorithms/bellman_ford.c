#include <stdio.h>
#include <stdlib.h>

#include "bellman_ford.h"
#include "graph_utils.h"

/*
Bellman-Ford com as duas otimizacoes discutidas em aula (ver bellman_ford.h):

  1. MEMORIA: um so vetor de estimativas M[v], atualizado no lugar
     (O(n), nao O(n^2) da tabela M[k][v]).
  2. TEMPO: M[v] so pode melhorar se algum predecessor u de v melhorou
     desde a ultima vez que as arestas de u foram relaxadas - as arestas de
     um vertice que nao mudou ja estao "em dia" (M[u] + peso >= M[v]).
     Entao cada passada relaxa SO as arestas de saida dos vertices ATIVOS
     (os que mudaram desde a ultima relaxacao deles), e o algoritmo para
     quando nao sobra ativo (a terminacao precoce vem de graca).

A passada continua percorrendo os vertices em ordem crescente e atualizando
no lugar: um vertice que melhora ja e usado mais adiante na MESMA passada.
Pular os inativos nao muda nada (relaxar uma aresta em dia nao altera M),
entao as distancias, a arvore e o numero de passadas sao os mesmos do laco
que relaxa todas as m arestas - so o trabalho cai: a passada custa O(n) de
varredura mais a soma dos graus de saida dos ativos, em vez de O(n + m). No
pior caso continua O(nm); em grafos reais a frente de ativos e pequena.

As arestas saem pelo iterador de vizinhanca - o mesmo do Dijkstra - entao o
algoritmo serve para as duas representacoes, direcionado ou nao.
*/
/*
O grafo dos pais (cada v aponta para parents[v]) tem um ciclo? Cada vertice
tem no maximo um pai, entao se sobe de cada vertice ainda nao visto marcando
o caminho com o numero da subida: chegar a um vertice marcado NESTA subida e
fechar um ciclo; chegar a -1 ou a um vertice de subida anterior, nao. Cada
vertice e marcado uma vez: O(n). `visita` (n inteiros) e reaproveitado entre
chamadas e zerado no fim.
*/
static bool ciclo_nos_pais(const int *parents, int n, int *visita) {
    bool achou = false;
    int subida = 0;
    for (int v = 0; v < n && !achou; v++) {
        if (visita[v] != 0 || parents[v] < 0) {
            continue;
        }
        subida += 1;
        int u = v;
        while (u >= 0 && visita[u] == 0) {
            visita[u] = subida;
            u = parents[u];
        }
        achou = (u >= 0 && visita[u] == subida);
    }
    for (int v = 0; v < n; v++) {
        visita[v] = 0;
    }
    return achou;
}

static int bellman_ford_nucleo(Graph *g, double *dist, int *parents, bool *ativo,
                               int n_ativos, bool *ciclo_out) {
    int n = g->n;
    int passadas = 0;

    /* n-1 passadas bastam: um caminho minimo (sem ciclo negativo no meio)
       tem no maximo n-1 arestas */
    int *visita = xcalloc((size_t)(n > 0 ? n : 1), sizeof(int));
    for (int i = 1; i <= n - 1 && n_ativos > 0; i++) {
        passadas = i;

        for (int u = 0; u < n; u++) {
            if (!ativo[u]) {
                continue; /* arestas de u em dia: relaxar nao muda nada */
            }
            ativo[u] = false;
            n_ativos -= 1;

            int v;
            double peso;
            NeighborIter it = neighbor_iter(g, u);
            while (neighbor_next(&it, &v, &peso)) {
                double alternativa = dist[u] + peso; /* M[v] no lugar */
                if (alternativa < dist[v]) {
                    dist[v] = alternativa;
                    parents[v] = u;
                    if (!ativo[v]) {
                        ativo[v] = true;
                        n_ativos += 1;
                    }
                }
            }
        }

        /* Deteccao ANTECIPADA de ciclo negativo (Kleinberg-Tardos, 6.27):
           se o grafo dos ponteiros de pai (v -> parents[v]) tem um ciclo, o
           ciclo e negativo. Conferir isso custa O(n) por passada - o mesmo
           da varredura dos ativos - e, num grafo com ciclo negativo,
           encerra o algoritmo em poucas passadas em vez de n-1. */
        if (n_ativos > 0 && ciclo_nos_pais(parents, n, visita)) {
            *ciclo_out = true;
            free(visita);
            return passadas;
        }
    }
    free(visita);

    /* Deteccao de ciclo negativo: depois de n-1 passadas, se ainda ha
       ativos, uma passada a mais so para flagar (com ciclo os valores nao
       sao confiaveis). So os ativos podem relaxar algo; se algum relaxa,
       existe caminho melhor com mais de n-1 arestas - um vertice repetido,
       isto e, um ciclo negativo alcancavel. Sem ativos = parou cedo. */
    bool ciclo_negativo = false;
    for (int u = 0; u < n && n_ativos > 0 && !ciclo_negativo; u++) {
        if (!ativo[u]) {
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

    *ciclo_out = ciclo_negativo;
    return passadas;
}

BellmanFordResult *bellman_ford(Graph *g, int fonte) {
    int n = g->n;

    double *dist = xmalloc((size_t)n * sizeof(double));
    int *parents = xmalloc((size_t)n * sizeof(int));
    for (int v = 0; v < n; v++) {
        dist[v] = INFINITO_PESO;
        parents[v] = -1; /* None no Python */
    }
    dist[fonte] = 0.0;

    /* na 1a passada, so a fonte esta ativa */
    bool *ativo = xcalloc((size_t)n, sizeof(bool));
    ativo[fonte] = true;

    bool ciclo_negativo = false;
    int passadas = bellman_ford_nucleo(g, dist, parents, ativo, 1, &ciclo_negativo);
    free(ativo);

    BellmanFordResult *result = xmalloc(sizeof(BellmanFordResult));
    result->n = n;
    result->dist = dist;
    result->parents = parents;
    result->negative_cycle = ciclo_negativo;
    result->passadas = passadas;
    return result;
}

BellmanFordResult *bellman_ford_potencial(Graph *g) {
    /*
    O "super-vertice": distancia de um vertice novo q, ligado a TODOS os
    vertices por aresta de peso 0, sem criar q de verdade - basta comecar
    com M[v] = 0 para todo v e todos ativos (a 1a passada a partir de q ja
    teria dado isso). Devolve h(v) = d(q, v) <= 0, o potencial do
    algoritmo de Johnson, e se o grafo tem ciclo negativo em QUALQUER
    lugar (todo ciclo e alcancavel a partir de q). Com q, o grafo tem n+1
    vertices: n passadas, nao n-1 - o nucleo faz n-1 passadas e a de
    deteccao, entao a 1a passada (a de q) ja conta como feita.
    */
    int n = g->n;
    double *dist = xmalloc((size_t)n * sizeof(double));
    int *parents = xmalloc((size_t)n * sizeof(int));
    bool *ativo = xmalloc((size_t)(n > 0 ? n : 1) * sizeof(bool));
    for (int v = 0; v < n; v++) {
        dist[v] = 0.0;
        parents[v] = -1;
        ativo[v] = true;
    }

    bool ciclo_negativo = false;
    int passadas = bellman_ford_nucleo(g, dist, parents, ativo, n, &ciclo_negativo);
    free(ativo);

    BellmanFordResult *result = xmalloc(sizeof(BellmanFordResult));
    result->n = n;
    result->dist = dist;
    result->parents = parents;
    result->negative_cycle = ciclo_negativo;
    result->passadas = passadas + 1; /* a passada a partir de q */
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
