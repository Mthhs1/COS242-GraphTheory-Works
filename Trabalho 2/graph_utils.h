#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include <stddef.h>

#include "graph.h"

/*
Utilitarios compartilhados pelos modulos de componentes, distancia e
estatisticas (graph_utils.py).

A biblioteca suporta as duas representacoes exigidas pelo enunciado
(matriz e lista de adjacencia). Para nao duplicar cada algoritmo, a
vizinhanca e acessada por uma unica porta: o iterador NeighborIter.

No Python neighbors() era um gerador (yield); em C ele virou esse
iterador, que percorre a lista encadeada ou a linha da matriz SEM ALOCAR
NADA. Por causa dele, as buscas, as componentes conexas, a distancia e o
diametro sao escritos uma vez so e valem para as duas representacoes.

Na Parte 2 (grafos com pesos) o iterador entrega tambem o peso da aresta:
o peso mora no no da lista ou na matriz de pesos do grafo, e nenhum
algoritmo precisa saber onde.

    NeighborIter it = neighbor_iter(g, v);
    int u;
    double peso;
    while (neighbor_next(&it, &u, &peso)) {
        ...
    }

Quem nao se interessa pelos pesos passa NULL no ultimo argumento.
*/

typedef struct NeighborIter {
    bool adj_list;
    Node *no;                /* lista: proximo no da vizinhanca   */
    const MatrixCell *linha; /* matriz: linha do vertice          */
    const double *linha_pesos; /* matriz: pesos da linha (NULL se o grafo nao tem pesos) */
    int coluna;              /* matriz: proxima coluna a examinar */
    int n;
} NeighborIter;

static inline NeighborIter neighbor_iter(Graph *g, int v) {
    NeighborIter it;
    it.adj_list = g->adj_list;
    it.n = g->n;
    it.coluna = 0;
    if (it.adj_list) {
        it.no = g->graph.list[v]->next;
        it.linha = NULL;
        it.linha_pesos = NULL;
    } else {
        it.no = NULL;
        it.linha = g->graph.matrix[v];
        it.linha_pesos = (g->weights != NULL) ? g->weights[v] : NULL;
    }
    return it;
}

/* true e o proximo vizinho em *out (e o peso da aresta em *peso_out, se
   pedido); false quando a vizinhanca acaba. peso_out pode ser NULL. */
static inline bool neighbor_next(NeighborIter *it, int *out, double *peso_out) {
    if (it->adj_list) {
        if (it->no == NULL) {
            return false;
        }
        *out = it->no->value;
        if (peso_out != NULL) {
            *peso_out = it->no->weight;
        }
        it->no = it->no->next;
        return true;
    }

    while (it->coluna < it->n) {
        int u = it->coluna;
        it->coluna += 1;
        if (it->linha[u] == 1) {
            *out = u;
            if (peso_out != NULL) {
                /* grafo sem pesos no arquivo: toda aresta vale 1 */
                *peso_out = (it->linha_pesos != NULL) ? it->linha_pesos[u] : 1.0;
            }
            return true;
        }
    }
    return false;
}

int degree(Graph *g, int v);
int *all_degrees(Graph *g);
int edge_count(Graph *g);

/* Auxiliares que em C substituem o gerenciamento automatico do Python */
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);

/* Concatenacao estilo `texto += f"..."` do Python; devolve o novo buffer */
char *str_append(char *buffer, const char *format, ...);

/*
Texto montado por muitas concatenacoes (arquivos de saida, representacao do
grafo). str_append mede o texto inteiro (strlen) e realoca a cada chamada, o
que deixa quadratico um laco com um append por vertice: nos grafos do estudo
de caso, o arquivo de saida levava segundos (grafo_3) ou horas (grafo_6).
StrBuf guarda tamanho e capacidade e dobra a capacidade quando falta espaco,
entao o custo total e linear no tamanho do texto.

    StrBuf texto = {NULL, 0, 0};
    strbuf_appendf(&texto, "%d ", v);
    char *resultado = strbuf_finish(&texto);  // nunca NULL; libere com free()
*/
typedef struct StrBuf {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

void strbuf_appendf(StrBuf *buffer, const char *format, ...);
char *strbuf_finish(StrBuf *buffer);

#endif
