#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

#include "node.h"

/*
graph.py no Python: a classe Graph carrega o grafo numa das duas
representacoes (self.graph + self.adj_list). Em C a representacao
interna vira uma uniao com o mesmo nome `graph`, e o numero de
vertices (len(graph) no Python) fica explicito no campo n.
*/

typedef struct Graph {
    int n;
    bool adj_list;
    union {
        Node **list;  /* self.graph quando adj_list == true  */
        int **matrix; /* self.graph quando adj_list == false */
    } graph;
} Graph;

/*
A tripla (ordem, pais, niveis) devolvida pelas buscas no Python.
parents guarda -1 onde o Python guardava None.
*/

typedef struct GraphSearch {
    int *result_discovered; /* primeiros `count` elementos = ordem de descoberta */
    int count;              /* len(result_discovered) no Python */
    int *parents;           /* -1 = sem pai (None no Python) */
    int *levels;            /* -1 = vertice nao alcancado */
} GraphSearch;

void GraphSearch_free(GraphSearch *search);

Graph *Graph_create(const char *archive, bool adj_list);
void Graph_destroy(Graph *g);
char *Graph_str(Graph *g);

/*
Buscas a partir de `start_node` (rotulo 1-based). Sem generate_tree
devolvem apenas a ordem (parents/levels ficam NULL); com generate_tree
devolvem a tripla completa. `target` e um rotulo 1-based, ou 0 para
nenhum (None no Python).
*/

GraphSearch *Graph_BFS(Graph *g, int start_node, int generate_tree, int target);
GraphSearch *Graph_DFS(Graph *g, int start_node, int generate_tree);

struct Components; /* definido em components.h */
struct GraphStats; /* definido em graph_stats.h */

/* Devolvem estruturas alocadas; o chamador libera com Components_free/free */
struct Components *Graph_connected_components(Graph *g);
int Graph_distance(Graph *g, int u, int v);
int Graph_diameter(Graph *g, int approximate);
struct GraphStats *Graph_stats(Graph *g);
char *Graph_write_output(Graph *g, const char *output_path);
char *Graph_write_search_tree(Graph *g, const char *output_path, int start_node, int use_dfs);

#endif
