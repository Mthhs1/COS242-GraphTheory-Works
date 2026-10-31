#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"
#include "heap.h"
#include "graph_utils.h"

/*
A abstracao pedida pelo enunciado ("tente nao implementar o algoritmo duas
vezes... abstraia a forma como as distancias sao armazenadas e atualizadas
e o minimo e extraido"). O algoritmo, la embaixo, e escrito UMA vez sobre
esta interface; as duas implementacoes ficam logo acima dele:

    DistStore store;
    dist_store_vetor(&store);            ou      dist_store_heap(&store);
    store.init(&store, n, source);
    while ((u = store.extract_min(&store)) != -1) { ... relaxa arestas ... }
    store.cleanup(&store);   // libera as estruturas internas; dist segue no resultado

  vetor: todo vertice comeca "ativo" com estimativa infinita; o minimo e
         achado por varredura (O(n) por extracao, O(n^2) no total).
  heap:  entram no heap so os vertices com estimativa finita; atualizar e o
         decrease-key do MinHeap (resolvido em O(log n) pelo posicao[]);
         extrair e a raiz do heap (O(log n), O(m log n) no total).

get e comum as duas (a estimativa de v), e set e "a nova estimativa de v",
seja la como a implementacao guarda.
*/
typedef struct DistStore {
    void (*init)(struct DistStore *s, int n, int source);
    double (*get)(struct DistStore *s, int v);
    void (*set)(struct DistStore *s, int v, double dist);
    int (*extract_min)(struct DistStore *s); /* -1 quando nao resta nada a fazer */
    void (*cleanup)(struct DistStore *s);

    /* campos das implementacoes; dist e comum */
    int n;
    double *dist;     /* estimativa atual de cada vertice */
    bool *ativo;      /* vetor: ainda nao extraido */
    bool *finalizado; /* heap e reinsercao: ja extraido (nao volta ao heap) */
    MinHeap heap;     /* heap: vertices com estimativa finita */

    /* reinsercao: heap de pares (chave, vertice) que cresce sob demanda */
    struct Entrada *fila;
    long fila_tam;
    long fila_cap;

    long pico;   /* maior numero de entradas no heap durante a execucao */
    long vazias; /* reinsercao: copias velhas descartadas na extracao */
} DistStore;

struct Entrada {
    double chave;
    int vertice;
};

/* ------------------------- implementacao: vetor ------------------------- */

static void vetor_init(DistStore *s, int n, int source) {
    s->n = n;
    s->dist = xmalloc((size_t)n * sizeof(double));
    s->ativo = xmalloc((size_t)n * sizeof(bool));
    for (int v = 0; v < n; v++) {
        s->dist[v] = INFINITO_PESO;
        s->ativo[v] = true;
    }
    s->dist[source] = 0.0;
}

static double comum_get(DistStore *s, int v) {
    return s->dist[v];
}

static void vetor_set(DistStore *s, int v, double dist) {
    s->dist[v] = dist; /* todo vertice ja esta "no vetor" desde o init */
}

static int vetor_extract_min(DistStore *s) {
    int melhor = -1;
    for (int v = 0; v < s->n; v++) {
        /* os ainda infinitos ficam para o fim: quando nao resta vertice com
           estimativa finita, o -1 encerra o algoritmo (o resto e inalcancavel) */
        if (s->ativo[v] && s->dist[v] != INFINITO_PESO &&
            (melhor == -1 || s->dist[v] < s->dist[melhor])) {
            melhor = v;
        }
    }
    if (melhor != -1) {
        s->ativo[melhor] = false;
    }
    return melhor;
}

static void vetor_cleanup(DistStore *s) {
    free(s->ativo);
}

/* ------------------------- implementacao: heap -------------------------- */

static void heap_store_init(DistStore *s, int n, int source) {
    s->n = n;
    s->dist = xmalloc((size_t)n * sizeof(double));
    s->finalizado = xmalloc((size_t)n * sizeof(bool));
    for (int v = 0; v < n; v++) {
        s->dist[v] = INFINITO_PESO;
        s->finalizado[v] = false;
    }
    s->dist[source] = 0.0;

    MinHeap_init(&s->heap, n);
    MinHeap_insert(&s->heap, source, 0.0); /* a fonte e a primeira estimativa finita */
    s->pico = 1;
}

static void heap_store_set(DistStore *s, int v, double dist) {
    s->dist[v] = dist;

    if (s->finalizado[v]) {
        return; /* com pesos >= 0, o algoritmo nunca relaxa quem ja saiu */
    }
    if (MinHeap_contains(&s->heap, v)) {
        MinHeap_decrease_key(&s->heap, v, dist);
    } else {
        MinHeap_insert(&s->heap, v, dist); /* primeira vez que v e alcancado */
        if (s->heap.tamanho > s->pico) {
            s->pico = s->heap.tamanho;
        }
    }
}

static int heap_store_extract_min(DistStore *s) {
    if (MinHeap_isEmpty(&s->heap)) {
        return -1;
    }
    int v = MinHeap_extract_min(&s->heap);
    s->finalizado[v] = true;
    return v;
}

static void heap_store_cleanup(DistStore *s) {
    free(s->finalizado);
    MinHeap_destroy(&s->heap);
}

/* ------------------ implementacao: reinsercao (saida facil) --------------
A dica do site: em vez de achar o vertice no heap para diminuir a chave,
insere-se uma NOVA entrada (chave, vertice) a cada melhora. A primeira vez
que o vertice sai do heap e a que vale; as outras sao copias velhas e sao
descartadas (finalizado[]). Nao precisa de posicao[], mas o heap guarda ate
uma entrada por relaxamento bem-sucedido (ate ~2m) em vez de uma por vertice.
Existe so para medir esse preco contra o heap com decrease-key.
*/

static void fila_push(DistStore *s, int v, double chave) {
    if (s->fila_tam == s->fila_cap) {
        s->fila_cap *= 2;
        s->fila = xrealloc(s->fila, (size_t)s->fila_cap * sizeof(struct Entrada));
    }
    /* sobe e desce com trocas, como o MinHeap: a comparacao mede so a
       diferenca de estrategia (reinserir x decrease-key) */
    long i = s->fila_tam++;
    s->fila[i].chave = chave;
    s->fila[i].vertice = v;
    while (i > 0) {
        long pai = (i - 1) / 2;
        if (s->fila[pai].chave <= s->fila[i].chave) {
            break;
        }
        struct Entrada t = s->fila[i];
        s->fila[i] = s->fila[pai];
        s->fila[pai] = t;
        i = pai;
    }
    if (s->fila_tam > s->pico) {
        s->pico = s->fila_tam;
    }
}

static struct Entrada fila_pop(DistStore *s) {
    struct Entrada topo = s->fila[0];
    s->fila[0] = s->fila[--s->fila_tam];
    long i = 0;
    for (;;) {
        long filho = 2 * i + 1;
        if (filho >= s->fila_tam) {
            break;
        }
        if (filho + 1 < s->fila_tam && s->fila[filho + 1].chave < s->fila[filho].chave) {
            filho += 1;
        }
        if (s->fila[i].chave <= s->fila[filho].chave) {
            break;
        }
        struct Entrada t = s->fila[i];
        s->fila[i] = s->fila[filho];
        s->fila[filho] = t;
        i = filho;
    }
    return topo;
}

static void reinsercao_init(DistStore *s, int n, int source) {
    s->n = n;
    s->dist = xmalloc((size_t)n * sizeof(double));
    s->finalizado = xmalloc((size_t)n * sizeof(bool));
    for (int v = 0; v < n; v++) {
        s->dist[v] = INFINITO_PESO;
        s->finalizado[v] = false;
    }
    s->dist[source] = 0.0;

    s->fila_cap = 1024;
    s->fila_tam = 0;
    s->fila = xmalloc((size_t)s->fila_cap * sizeof(struct Entrada));
    fila_push(s, source, 0.0);
}

static void reinsercao_set(DistStore *s, int v, double dist) {
    s->dist[v] = dist;
    if (!s->finalizado[v]) {
        fila_push(s, v, dist); /* a entrada antiga de v fica no heap, velha */
    }
}

static int reinsercao_extract_min(DistStore *s) {
    while (s->fila_tam > 0) {
        struct Entrada e = fila_pop(s);
        if (!s->finalizado[e.vertice]) {
            s->finalizado[e.vertice] = true;
            return e.vertice;
        }
        s->vazias += 1; /* copia velha: o vertice ja saiu com chave menor */
    }
    return -1;
}

static void reinsercao_cleanup(DistStore *s) {
    free(s->finalizado);
    free(s->fila);
}

/* ----------------------------- o algoritmo ------------------------------ */

static void dist_store_vetor(DistStore *s) {
    s->init = vetor_init;
    s->get = comum_get;
    s->set = vetor_set;
    s->extract_min = vetor_extract_min;
    s->cleanup = vetor_cleanup;
    s->n = 0;
    s->dist = NULL;
    s->ativo = NULL;
    s->finalizado = NULL;
}

static void dist_store_heap(DistStore *s) {
    s->init = heap_store_init;
    s->get = comum_get;
    s->set = heap_store_set;
    s->extract_min = heap_store_extract_min;
    s->cleanup = heap_store_cleanup;
    s->n = 0;
    s->dist = NULL;
    s->ativo = NULL;
    s->finalizado = NULL;
}

static void dist_store_reinsercao(DistStore *s) {
    s->init = reinsercao_init;
    s->get = comum_get;
    s->set = reinsercao_set;
    s->extract_min = reinsercao_extract_min;
    s->cleanup = reinsercao_cleanup;
    s->n = 0;
    s->dist = NULL;
    s->ativo = NULL;
    s->finalizado = NULL;
}

DijkstraResult *dijkstra(Graph *g, int source, DijkstraModo modo) {
    if (g->negative_weight) {
        fprintf(stderr,
                "o grafo tem aresta(s) com peso negativo: a biblioteca ainda "
                "nao implementa caminhos minimos com pesos negativos\n");
        return NULL;
    }

    DistStore store;
    if (modo == DIJKSTRA_HEAP) {
        dist_store_heap(&store);
    } else if (modo == DIJKSTRA_HEAP_REINSERCAO) {
        dist_store_reinsercao(&store);
    } else {
        dist_store_vetor(&store);
    }
    store.pico = 0;
    store.vazias = 0;
    store.init(&store, g->n, source);

    int *parents = xmalloc((size_t)g->n * sizeof(int));
    for (int v = 0; v < g->n; v++) {
        parents[v] = -1; /* None no Python */
    }

    int u;
    while ((u = store.extract_min(&store)) != -1) {
        double du = store.get(&store, u);

        int v;
        double peso;
        NeighborIter it = neighbor_iter(g, u);
        while (neighbor_next(&it, &v, &peso)) {
            double alternativa = du + peso;
            if (alternativa < store.get(&store, v)) {
                store.set(&store, v, alternativa);
                parents[v] = u;
            }
        }
    }

    /* dist segue no resultado; o resto pertence a implementacao */
    store.cleanup(&store);

    DijkstraResult *result = xmalloc(sizeof(DijkstraResult));
    result->dist = store.dist;
    result->parents = parents;
    result->heap_pico = store.pico;
    result->extracoes_vazias = store.vazias;
    return result;
}

int *dijkstra_path(DijkstraResult *result, int alvo, int *comprimento_out) {
    if (result->dist[alvo] == INFINITO_PESO) {
        *comprimento_out = 0;
        return NULL;
    }

    /* caminha pela arvore de pais (alvo -> fonte) e inverte */
    int comprimento = 0;
    for (int v = alvo; v != -1; v = result->parents[v]) {
        comprimento += 1;
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

void DijkstraResult_free(DijkstraResult *result) {
    if (result == NULL) {
        return;
    }
    free(result->dist);
    free(result->parents);
    free(result);
}

#ifdef DIJKSTRA_MAIN
/* if __name__ == "__main__" do dijkstra: distancias e caminhos no exemplo
   da Figura 1 da Parte 2 (sem o peso negativo), nas duas formas de guardar
   as estimativas */
#include <stdio.h>

int main(void) {
    for (int modo_i = 0; modo_i <= 1; modo_i++) {
        DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
        printf("--- %s ---\n", modo_i ? "heap" : "vetor");

        Graph *g = Graph_create("graph_exemplo_pesos_positivos.txt", true);
        DijkstraResult *r = dijkstra(g, 0, modo);
        if (r == NULL) {
            printf("(grafo com pesos negativos)\n");
            Graph_destroy(g);
            continue;
        }

        for (int v = 1; v < g->n; v++) { /* vertices 2..5 em rotulos 1-based */
            int comprimento;
            int *caminho = dijkstra_path(r, v, &comprimento);
            printf("d(1,%d) = %g  caminho: 1", v + 1, r->dist[v]);
            for (int i = 1; i < comprimento; i++) {
                printf(" -> %d", caminho[i] + 1);
            }
            printf("\n");
            free(caminho);
        }

        DijkstraResult_free(r);
        Graph_destroy(g);
    }
    return 0;
}
#endif
