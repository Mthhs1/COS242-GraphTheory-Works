#ifndef DFS_H
#define DFS_H

#include "graph.h"

/*
DFS.py: busca em profundidade nas duas representacoes. Sempre devolvem
a tripla (ordem, pais, niveis) como GraphSearch.
*/

GraphSearch *DFS_adj_list(Node **graph, int n, int start_node);
GraphSearch *DFS_matrix(int **graph, int n, int start_node);

#endif
