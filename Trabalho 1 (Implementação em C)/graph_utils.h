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

No Python neighbors() e um gerador (yield); em C devolve um vetor
alocado com os vizinhos e o tamanho em *count_out (o chamador libera).
*/

int *neighbors(Graph *g, int v, int *count_out);
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
