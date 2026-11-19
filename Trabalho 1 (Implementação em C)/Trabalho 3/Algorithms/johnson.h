#ifndef JOHNSON_H
#define JOHNSON_H

#include "graph.h"
#include "dijkstra.h"

/*
Parte 3 (otimizacao): distancia entre TODOS OS PARES com pesos negativos pelo
algoritmo de JOHNSON, em O(nm log n) - contra O(n^3) do Floyd-Warshall e
O(n^2 m) de n execucoes do Bellman-Ford. Em grafo esparso (m ~ n) e quase
um fator n mais rapido que os dois.

A ideia: o Dijkstra e rapido mas nao aceita peso negativo. Um POTENCIAL h
(um numero por vertice) transforma cada peso em

    peso'(u, v) = peso(u, v) + h(u) - h(v)

e todo caminho s ~> t muda pelo MESMO valor h(s) - h(t) (os termos do meio
se cancelam), entao os caminhos minimos sao os mesmos. Com h(v) = distancia
de um vertice virtual ligado a todos por peso 0 (um Bellman-Ford so, em
bellman_ford_potencial), a desigualdade triangular h(v) <= h(u) + peso(u,v)
da peso' >= 0, e o Dijkstra com heap passa a valer em cada fonte:

    d(s, t) = d'(s, t) - h(s) + h(t)

O Bellman-Ford do potencial tambem detecta ciclo negativo em QUALQUER parte
do grafo (o vertice virtual alcanca todos); com ciclo negativo nao ha
distancias e johnson_fonte nao deve ser chamada.
*/

typedef struct Johnson {
    int n;
    bool negative_cycle; /* ciclo negativo em qualquer parte do grafo */
    double *h;           /* o potencial: h[v] = d(virtual, v) <= 0     */
    int passadas;        /* passadas do Bellman-Ford do potencial      */
} Johnson;

/* O passo O(nm): o Bellman-Ford do potencial. Libere com Johnson_free. */
Johnson *johnson_preparar(Graph *g);

/* Uma fonte (indice 0-based), O(m log n): Dijkstra com heap nos pesos
   reduzidos, com dist ja CORRIGIDA para as distancias verdadeiras e a arvore
   de caminhos minimos em parents. Libere com DijkstraResult_free. */
DijkstraResult *johnson_fonte(Johnson *j, Graph *g, int fonte);

void Johnson_free(Johnson *j);

#endif
