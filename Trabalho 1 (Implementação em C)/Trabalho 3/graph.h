#ifndef GRAPH_H
#define GRAPH_H

#include <math.h>
#include <stdbool.h>

#include "node.h"

/*
graph.py no Python: a classe Graph carrega o grafo numa das duas
representacoes (self.graph + self.adj_list). Em C a representacao
interna vira uma uniao com o mesmo nome `graph`, e o numero de
vertices (len(graph) no Python) fica explicito no campo n.
*/

/*
Distancia "infinita" para vertices nao alcancados, compartilhada pelo
Dijkstra (Parte 2) e pelo Bellman-Ford e Floyd-Warshall (Parte 3). O
INFINITO da Parte 1 (-1) nao serve aqui: em grafo com pesos, -1 pode ser
distancia valida. (Na Parte 2 morava no dijkstra.h; ganhou a propria casa
quando os novos algoritmos tambem precisaram dele.)
*/
#define INFINITO_PESO HUGE_VAL

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
    bool directed;         /* Parte 3: arestas lidas na direcao a -> b     */
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

Graph *Graph_create(const char *archive, bool adj_list, bool direcionado);
void Graph_destroy(Graph *g);
char *Graph_str(Graph *g);

/*
Parte 3: o GRAFO TRANSPOSTO (todas as arestas invertidas). E o ingrediente
do Bellman-Ford "de todos para um": a distancia de v ate o alvo t no grafo
e a distancia de t ate v no transposto. Em grafo NAO direcionado o
transposto e uma copia (as arestas ja existem nas duas direcoes).
*/
Graph *Graph_reverse(Graph *g);

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
struct DijkstraResult;   /* definido em dijkstra.h (Parte 2) */
struct BellmanFordResult; /* definido em bellman_ford.h (Parte 3) */
struct FloydWarshallResult; /* definido em floyd_warshall.h (Parte 3) */

/* Devolvem estruturas alocadas; o chamador libera com Components_free/free.

   ATENCAO (Parte 3): componentes conexas, distancia, diametro e
   estatisticas sao definidas para grafos NAO direcionados; chamadas em
   grafo direcionado encerram o programa com mensagem de erro. As buscas
   (Graph_BFS/Graph_DFS), o Dijkstra, o Bellman-Ford e o Floyd-Warshall
   valem para os dois. */
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

/*
Parte 3: Bellman-Ford. Com pesos negativos, sem recusar: distancias e
arvore de caminhos minimos, e se existe ciclo negativo.

    Graph_bellman_ford(g, fonte)        distancia de `fonte` (rotulo 1-based)
                                        a todos os vertices
    Graph_bellman_ford_to(g, alvo)      distancia de TODOS os vertices ao
                                        `alvo` (o que o enunciado pede),
                                        sobre o grafo transposto
*/
struct BellmanFordResult *Graph_bellman_ford(Graph *g, int fonte);
struct BellmanFordResult *Graph_bellman_ford_to(Graph *g, int alvo);

/*
Parte 3: Floyd-Warshall: distancia e arvore de caminhos minimos entre
TODOS os pares de vertices, e se o grafo tem ciclo negativo.
*/
struct FloydWarshallResult *Graph_floyd_warshall(Graph *g);

#endif
