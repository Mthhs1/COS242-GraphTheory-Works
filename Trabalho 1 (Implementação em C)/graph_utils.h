#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include <stddef.h>

#include "graph.h"

/*
Utilitarios compartilhados pelos modulos de componentes, distancia e
estatisticas (graph_utils.py).

A biblioteca suporta as duas representacoes exigidas pelo enunciado
(matriz e lista de adjacencia). Para nao duplicar cada algoritmo, tudo
aqui usa a flag `adj_list` do grafo e neighbors() como unica porta de
acesso a vizinhanca.

No Python neighbors() e um gerador (yield). Em C o gerador virou o
iterador NeighborIter: ele percorre a lista encadeada ou a linha da
matriz SEM ALOCAR NADA, e e a unica porta de acesso a vizinhanca. Por
causa dele, as buscas, as componentes conexas, a distancia e o diametro
sao escritos uma vez so e valem para as duas representacoes.

    NeighborIter it = neighbor_iter(g, v);
    int u;
    while (neighbor_next(&it, &u)) {
        ...
    }
*/

typedef struct NeighborIter {
    bool adj_list;
    Node *no;                /* lista: proximo no da vizinhanca   */
    const MatrixCell *linha; /* matriz: linha do vertice          */
    int coluna;              /* matriz: proxima coluna a examinar */
    int n;
} NeighborIter;

static inline NeighborIter neighbor_iter(Graph *g, int v) {
    NeighborIter it;
    it.adj_list = g->adj_list;
    it.n = g->n;
    it.coluna = 0;
    it.no = it.adj_list ? g->graph.list[v]->next : NULL;
    it.linha = it.adj_list ? NULL : g->graph.matrix[v];
    return it;
}

/* true e o proximo vizinho em *out; false quando a vizinhanca acaba */
static inline bool neighbor_next(NeighborIter *it, int *out) {
    if (it->adj_list) {
        if (it->no == NULL) {
            return false;
        }
        *out = it->no->value;
        it->no = it->no->next;
        return true;
    }

    while (it->coluna < it->n) {
        int u = it->coluna;
        it->coluna += 1;
        if (it->linha[u] == 1) {
            *out = u;
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
