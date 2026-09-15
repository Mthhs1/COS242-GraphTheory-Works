#ifndef BFS_H
#define BFS_H

#include "graph.h"

/*
BFS.py: busca em largura nas duas representacoes. Sempre devolvem a
tripla (ordem, pais, niveis) como GraphSearch. `target` e um indice
0-based, ou -1 para nenhum (None no Python); quando o alvo e alcancado,
a busca encerra assim que ele sai da fila.
*/

GraphSearch *BFS_adj_list(Node **graph, int n, int start_node, int target);
GraphSearch *BFS_matrix(int **graph, int n, int start_node, int target);

#endif
