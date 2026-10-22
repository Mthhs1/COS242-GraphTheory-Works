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

/*
Celula da matriz de adjacencia: guarda 0 ou 1, entao 1 byte basta. Com int
(4 bytes) a matriz do grafo_2 do estudo de caso precisaria de 9,3 GB e nao
cabia na memoria da maquina; com 1 byte sao 2,3 GB. Como o consumo cresce
com n^2, cada byte por celula decide se o grafo cabe ou nao.
*/
typedef unsigned char MatrixCell;

/*
Parte 2: grafos com pesos nas arestas (enunciado, Parte 2, item 1). Na lista,
o peso viaja no proprio no (Node.weight). Na matriz, a adjacencia continua
em 1 byte por celula e os pesos ficam numa matriz double separada, alocada
SO se o arquivo tem algum peso diferente de 1 (senao ficaria NULL e toda
aresta vale 1, como na Parte 1).
*/
typedef struct Graph {
    int n;
    bool adj_list;
    bool weighted;         /* true se o arquivo tem algum peso != 1 */
    bool negative_weight;  /* true se alguma aresta tem peso < 0: Dijkstra recusa */
    double **weights;      /* so na matriz: peso de cada aresta; NULL se sem pesos */
    union {
        Node **list;         /* self.graph quando adj_list == true  */
        MatrixCell **matrix; /* self.graph quando adj_list == false */
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

struct Components;   /* definido em components.h */
struct GraphStats;   /* definido em graph_stats.h */
struct DijkstraResult; /* definido em dijkstra.h (Parte 2) */

/* Devolvem estruturas alocadas; o chamador libera com Components_free/free */
struct Components *Graph_connected_components(Graph *g);
int Graph_distance(Graph *g, int u, int v);
int Graph_diameter(Graph *g, int approximate);
struct GraphStats *Graph_stats(Graph *g);
char *Graph_write_output(Graph *g, const char *output_path);
char *Graph_write_search_tree(Graph *g, const char *output_path, int start_node, int use_dfs);

/*
Parte 2: Dijkstra a partir de `start_node` (rotulo 1-based), com estimativas
em vetor (use_heap == 0) ou em heap (use_heap == 1). Devolve NULL, com
mensagem, se o grafo tem aresta com peso negativo.
*/
struct DijkstraResult *Graph_dijkstra(Graph *g, int start_node, int use_heap);

#endif
