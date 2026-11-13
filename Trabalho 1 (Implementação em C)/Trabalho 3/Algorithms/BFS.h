#ifndef BFS_H
#define BFS_H

#include "graph.h"

/*
BFS.py: busca em largura. UMA implementacao para as duas representacoes -
a vizinhanca vem do iterador de graph_utils.h, que sabe percorrer tanto a
lista encadeada quanto a linha da matriz.

Devolve a tripla (ordem, pais, niveis) como GraphSearch. `target` e um
indice 0-based, ou -1 para nenhum (None no Python); quando o alvo e
alcancado, a busca encerra assim que ele sai da fila.
*/

GraphSearch *BFS(Graph *g, int start_node, int target);

#endif
