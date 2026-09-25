#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "BFS.h"
#include "DFS.h"
#include "graph_gen.h"
#include "graph_utils.h"
#include "components.h"
#include "distance.h"
#include "graph_stats.h"

void GraphSearch_free(GraphSearch *search) {
    if (search == NULL) {
        return;
    }
    free(search->result_discovered);
    free(search->parents);
    free(search->levels);
    free(search);
}

Graph *Graph_create(const char *archive, bool adj_list) {
    /*
    Carrega o grafo do arquivo `archive`. adj_list=true usa lista de
    adjacencia; adj_list=false (padrao no Python) usa matriz.
    */
    Graph *g = xmalloc(sizeof(Graph));
    g->adj_list = adj_list;
    g->n = 0;

    if (adj_list) {
        g->graph.list = create_list_graph(archive, &g->n);
        return g;
    }

    g->graph.matrix = create_matrix_graph(archive, &g->n);
    return g;
}

void Graph_destroy(Graph *g) {
    if (g == NULL) {
        return;
    }
    if (g->adj_list) {
        free_list_graph(g->graph.list, g->n);
    } else {
        free_matrix_graph(g->graph.matrix, g->n);
    }
    free(g);
}

/* _to_index no Python: privada da classe (static em C) */
static int Graph_to_index(Graph *g, int label) {
    /* Converte rotulo 1-based (como no arquivo de entrada) em indice
       0-based, validando os limites. No lugar do ValueError do Python,
       encerra o programa com mensagem de erro. */
    int index = label - 1;
    if (!(0 <= index && index < g->n)) {
        fprintf(stderr, "vertice invalido: %d (o grafo tem %d vertices)\n", label, g->n);
        exit(EXIT_FAILURE);
    }
    return index;
}

char *Graph_str(Graph *g) {
    /* __str__ no Python */
    if (g->adj_list) {
        return str_adj_list(g->graph.list, g->n);
    }
    return str_matrix(g->graph.matrix, g->n);
}

GraphSearch *Graph_BFS(Graph *g, int start_node, int generate_tree, int target) {
    int start = Graph_to_index(g, start_node);
    int target_index = (target == 0) ? -1 : Graph_to_index(g, target);

    GraphSearch *search = BFS(g, start, target_index);

    if (!generate_tree) {
        /* devolve apenas a ordem de descoberta, como generate_tree=False */
        free(search->parents);
        free(search->levels);
        search->parents = NULL;
        search->levels = NULL;
    }
    return search;
}

GraphSearch *Graph_DFS(Graph *g, int start_node, int generate_tree) {
    int start = Graph_to_index(g, start_node);

    GraphSearch *search = DFS(g, start);

    if (!generate_tree) {
        free(search->parents);
        free(search->levels);
        search->parents = NULL;
        search->levels = NULL;
    }
    return search;
}

/* ------------------------------------------------------------------ */
/* Requisitos 2, 5 e 6 do enunciado.                                   */
/* ------------------------------------------------------------------ */

struct Components *Graph_connected_components(Graph *g) {
    /* Componentes conexas, em ordem decrescente de tamanho (rotulos 1-based). */
    struct Components *components = xmalloc(sizeof(struct Components));
    *components = connected_components(g);
    return components;
}

int Graph_distance(Graph *g, int u, int v) {
    /* Distancia entre os vertices u e v (rotulos 1-based). -1 se nao houver caminho. */
    Graph_to_index(g, u);
    Graph_to_index(g, v);
    return distance(g, u, v, true);
}

int Graph_diameter(Graph *g, int approximate) {
    /* Diametro do grafo. approximate=true usa a heuristica de dupla varredura. */
    if (approximate) {
        return approximate_diameter(g);
    }
    return diameter(g);
}

struct GraphStats *Graph_stats(Graph *g) {
    /* n, m, grau minimo/maximo/medio e mediana de grau. */
    struct GraphStats *stats = xmalloc(sizeof(struct GraphStats));
    *stats = graph_stats(g);
    return stats;
}

char *Graph_write_output(Graph *g, const char *output_path) {
    /* Requisito 2: arquivo de saida com estatisticas e componentes conexas. */
    return write_output(g, output_path);
}

char *Graph_write_search_tree(Graph *g, const char *output_path, int start_node, int use_dfs) {
    /* Requisito 4: grava pai e nivel de cada vertice da arvore de busca. */
    GraphSearch *search;
    if (use_dfs) {
        search = Graph_DFS(g, start_node, 1);
    } else {
        search = Graph_BFS(g, start_node, 1, 0);
    }

    char *content = write_search_tree(search->parents, search->levels, g->n,
                                       output_path, start_node - 1);
    GraphSearch_free(search);
    return content;
}

#ifdef GRAPH_MAIN
/* if __name__ == "__main__" do graph.py. Os caminhos assumem que o
   executavel roda a partir desta pasta (graph1.txt fica na raiz do repo,
   como em parents[1] / "graph1.txt" no Python). */

static void imprimir_representacao(Graph *g) {
    /* grafos dos estudos de caso sao grandes demais para imprimir a
       representacao inteira (e nao faz sentido no terminal) */
    if (g->n > 1000) {
        printf("(grafo com %d vertices: representacao nao impressa)\n", g->n);
        return;
    }
    char *text = Graph_str(g);
    printf("%s\n", text);
    free(text);
}

static void imprimir_busca(const char *titulo, GraphSearch *search) {
    printf("%s: [", titulo);
    int limite = (search->count < 20) ? search->count : 20;
    for (int i = 0; i < limite; i++) {
        printf(i == 0 ? "%d" : ", %d", search->result_discovered[i]);
    }
    if (search->count > limite) {
        printf(", ... (%d vertices no total)", search->count);
    }
    printf("]\n");
}

int main(void) {
    const char *graph_file = "../graph1.txt";

    printf("\nGrafo 1 - Matriz de Adjacência\n");
    Graph *graph1 = Graph_create(graph_file, false);
    imprimir_representacao(graph1);

    GraphSearch *bfs1 = Graph_BFS(graph1, 1, 0, 0);
    imprimir_busca("BFS a partir do vertice 1", bfs1);
    GraphSearch_free(bfs1);

    GraphSearch *dfs1 = Graph_DFS(graph1, 1, 0);
    imprimir_busca("DFS a partir do vertice 1", dfs1);
    GraphSearch_free(dfs1);
    Graph_destroy(graph1);

    printf("\nGrafo 1 - Lista de Adjacência\n");
    Graph *graph2 = Graph_create(graph_file, true);
    imprimir_representacao(graph2);

    GraphSearch *bfs2 = Graph_BFS(graph2, 1, 0, 0);
    imprimir_busca("BFS a partir do vertice 1", bfs2);
    GraphSearch_free(bfs2);

    GraphSearch *dfs2 = Graph_DFS(graph2, 1, 0);
    imprimir_busca("DFS a partir do vertice 1", dfs2);
    GraphSearch_free(dfs2);

    printf("Distancia entre os vertices 1 e 4: %d\n", Graph_distance(graph2, 1, 4));
    Graph_destroy(graph2);
    return 0;
}
#endif
