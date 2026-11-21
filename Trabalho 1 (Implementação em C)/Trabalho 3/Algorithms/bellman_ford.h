#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H

#include "graph.h"

/*
Parte 3 do enunciado: distancia e caminho minimo COM PESOS NEGATIVOS,
pelo Bellman-Ford - o que o Dijkstra da Parte 2 recusa.

O enunciado pede o problema "de todos para um": a distancia e o caminho
minimo DE TODOS OS VERTICES para um vertice especifico. O truque e o
grafo transposto: a distancia de v ate o alvo t no grafo e a distancia
de t ate v no transposto, onde o Bellman-Ford e o de sempre (uma fonte
so). Por isso a biblioteca expoe as duas pontas:

    bellman_ford(g, fonte)       fonte -> todos (o classico; tambem serve
                                 para o "todos os pares" do estudo de caso,
                                 uma chamada por fonte)
    bellman_ford_to(g, alvo)     todos -> alvo (o que o enunciado pede),
                                 sobre o transposto

As duas otimizacoes discutidas em aula:

  1. MEMORIA: a programacao dinamica "obvia" guardaria M[k][v], a
     distancia minima da fonte a v com no maximo k arestas, para todos os
     k - O(n^2) de memoria. Basta o vetor M[v] das estimativas CORRENTES,
     atualizado no lugar: cada relaxamento ja le o valor mais novo, o que
     so antecipa melhoras. O(n) de memoria, como o Dijkstra.

  2. TEMPO - SO OS VERTICES ATIVOS: M[v] so pode melhorar se algum
     predecessor melhorou na passada anterior. Cada passada relaxa so as
     arestas de saida dos vertices que mudaram na passada anterior (os
     ativos), em vez das m arestas. Quando nenhum vertice muda, a lista
     de ativos fica vazia e o algoritmo para antes das n-1 passadas (a
     TERMINACAO PRECOCE). (Se existisse um ciclo negativo alcancavel,
     nenhuma passada seria a ultima: sempre daria para melhorar girando o
     ciclo. Entao parar cedo nunca "esconde" ciclo negativo.)

CICLO NEGATIVO: apos as passadas, mais uma passada de deteccao - se ela
consegue relaxar alguma aresta, existe um caminho melhor com mais de n-1
arestas, o so e possivel com um vertice repetido, isto e, um ciclo
negativo. A deteccao e no que o algoritmo ALCANCA: bellman_ford flaga os
ciclos negativos alcancaveis DA FONTE; bellman_ford_to, os que CHEGAM ao
alvo (alcancaveis da fonte no transposto). Nao e uma limitacao: ciclo
que a fonte nao alcanca (ou que nao chega ao alvo) nao participa de
nenhum caminho do problema. Com negative_cycle == true, dist/parents nao
contem valores confiaveis (girando o ciclo, a distancia cai para sempre).

A arvore de caminhos minimos:

  bellman_ford:   parents[v] = o vertice ANTES de v no caminho fonte->v
                  (a arvore enraizada na fonte, como a do Dijkstra);
                  -1 = raiz ou inalcancavel.
  bellman_ford_to: parents[v] = o vertice DEPOIS de v no caminho v->alvo
                  (o pai de v no transposto = o primeiro passo de v rumo
                  ao alvo; a arvore "aponta para" o alvo); -1 = alvo ou
                  inalcancavel.
*/

typedef struct BellmanFordResult {
    int n;               /* numero de vertices (tamanho de dist/parents) */
    double *dist;        /* dist[v] = INFINITO_PESO se inalcancavel */
    int *parents;        /* a arvore de caminhos minimos (ver acima) */
    bool negative_cycle; /* ciclo negativo no que o problema alcanca */
    int passadas;        /* passadas de relaxamento executadas (<= n-1):
                            o ganho da terminacao precoce */
} BellmanFordResult;

void BellmanFordResult_free(BellmanFordResult *result);

/* Fonte (indice 0-based) -> todos os vertices. */
BellmanFordResult *bellman_ford(Graph *g, int fonte);

/* Todos os vertices -> alvo (indice 0-based), sobre o grafo transposto. */
BellmanFordResult *bellman_ford_to(Graph *g, int alvo);

/* O potencial do algoritmo de Johnson: h(v) = distancia de um vertice
   virtual ligado a todos por arestas de peso 0 (todos comecam em 0 e
   ativos). negative_cycle diz se ha ciclo negativo EM QUALQUER PARTE do
   grafo. parents nao tem significado de arvore aqui. */
BellmanFordResult *bellman_ford_potencial(Graph *g);

/* Caminho minimo fonte -> alvo do resultado de bellman_ford (0-based), em
   ordem fonte -> alvo, com o comprimento em *comprimento_out. NULL (e 0)
   se alvo nao e alcancavel. Libere com free(). */
int *bellman_ford_path(BellmanFordResult *result, int alvo, int *comprimento_out);

/* Caminho minimo origem -> alvo do resultado de bellman_ford_to (0-based),
   em ordem origem -> alvo, com o comprimento em *comprimento_out. NULL (e
   0) se origem nao alcanca o alvo. Libere com free(). */
int *bellman_ford_path_to(BellmanFordResult *result, int origem, int *comprimento_out);

#endif
