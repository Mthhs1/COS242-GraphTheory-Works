#ifndef DFS_H
#define DFS_H

#include "graph.h"

/*
DFS.py: busca em profundidade. UMA implementacao para as duas
representacoes - a vizinhanca vem do iterador de graph_utils.h.

Devolve a tripla (ordem, pais, niveis) como GraphSearch.
*/

GraphSearch *DFS(Graph *g, int start_node);

#endif
