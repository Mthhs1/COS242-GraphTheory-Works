#ifndef DISTANCE_H
#define DISTANCE_H

#include "graph.h"

/*
Requisito 5 do enunciado: distancia entre dois vertices e diametro.

A distancia usa a BFS como primitiva, como pede o enunciado: em grafo nao
ponderado, o nivel da BFS E a distancia minima.

Diametro exato: BFS a partir de cada vertice, O(n * (n + m)). Inviavel nos
grafos grandes do estudo de caso, por isso ha tambem a versao aproximada.

Diametro aproximado (heuristica de dupla varredura): a partir de um vertice
qualquer, a BFS acha o vertice mais distante u; uma segunda BFS a partir de u
da uma estimativa por baixo do diametro. E barato (2 BFS por componente) e
costuma acertar ou chegar muito perto em grafos reais.
*/

#define INFINITO -1 /* marcador de "inalcancavel", coerente com levels = -1 do BFS.c */

int *bfs_distances(Graph *g, int source);
int distance(Graph *g, int u, int v, bool one_based);
int diameter(Graph *g);
int approximate_diameter(Graph *g);

#endif
