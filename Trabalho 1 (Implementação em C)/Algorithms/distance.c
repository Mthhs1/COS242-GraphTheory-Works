#include <stdlib.h>

#include "distance.h"
#include "components.h"
#include "my_queue.h"
#include "graph_utils.h"

int *bfs_distances(Graph *g, int source) {
    /*
    Distancias de `source` (0-based) para todos os vertices.
    Retorna vetor onde nao alcancado = INFINITO.
    */
    int n = g->n;
    int *distances = xmalloc((size_t)n * sizeof(int));
    for (int v = 0; v < n; v++) {
        distances[v] = INFINITO;
    }
    distances[source] = 0;

    Queue queue;
    Queue_init(&queue);
    Queue_enqueue(&queue, source);

    while (!Queue_isEmpty(&queue)) {
        int current = Queue_dequeue(&queue);

        int neighbor_count;
        int *neighbor = neighbors(g, current, &neighbor_count);
        for (int j = 0; j < neighbor_count; j++) {
            if (distances[neighbor[j]] == INFINITO) {
                distances[neighbor[j]] = distances[current] + 1;
                Queue_enqueue(&queue, neighbor[j]);
            }
        }
        free(neighbor);
    }

    return distances;
}

int distance(Graph *g, int u, int v, bool one_based) {
    /*
    Distancia entre u e v. Por padrao recebe rotulos 1-based (como no arquivo
    de entrada) e devolve INFINITO se estiverem em componentes diferentes.
    */
    if (one_based) {
        u -= 1;
        v -= 1;
    }

    int *distances = bfs_distances(g, u);
    int result = distances[v];
    free(distances);
    return result;
}

/* _eccentricity no Python: privada do modulo (static em C) */
static void eccentricity(Graph *g, int source, int *best_distance_out, int *best_vertex_out) {
    /* Maior distancia finita a partir de source, e o vertice que a realiza. */
    int *distances = bfs_distances(g, source);
    int best_vertex = source;
    int best_distance = 0;

    for (int vertex = 0; vertex < g->n; vertex++) {
        if (distances[vertex] != INFINITO && distances[vertex] > best_distance) {
            best_vertex = vertex;
            best_distance = distances[vertex];
        }
    }

    free(distances);
    *best_distance_out = best_distance;
    *best_vertex_out = best_vertex;
}

int diameter(Graph *g) {
    /*
    Diametro exato: maior distancia minima entre qualquer par alcancavel.
    Em grafo desconexo, e o maior diametro entre as componentes (distancias
    infinitas sao ignoradas).
    */
    int largest = 0;
    for (int source = 0; source < g->n; source++) {
        int eccentricity_value;
        int vertex;
        eccentricity(g, source, &eccentricity_value, &vertex);
        if (eccentricity_value > largest) {
            largest = eccentricity_value;
        }
    }
    return largest;
}

int approximate_diameter(Graph *g) {
    /*
    Estimativa por dupla varredura, aplicada uma vez por componente.
    O valor devolvido e sempre <= diametro real (e uma cota inferior).
    */
    Components components = connected_components(g);

    int estimate = 0;
    for (int i = 0; i < components.count; i++) {
        int start = components.components[i][0] - 1; /* rotulo 1-based -> indice */

        int primeira;
        int farthest;
        eccentricity(g, start, &primeira, &farthest);

        int second_pass;
        int vertex;
        eccentricity(g, farthest, &second_pass, &vertex);

        if (second_pass > estimate) {
            estimate = second_pass;
        }
    }

    Components_free(&components);
    return estimate;
}

#ifdef DISTANCE_MAIN
/* if __name__ == "__main__" do distance.py */
#include <stdio.h>

int main(void) {
    for (int use_list = 0; use_list <= 1; use_list++) {
        const char *rep = use_list ? "lista de adjacencia" : "matriz de adjacencia";
        Graph *g = Graph_create("graph_exemplo.txt", use_list != 0);
        printf("--- %s ---\n", rep);
        printf("d(1,2) = %d\n", distance(g, 1, 2, true));
        printf("d(1,3) = %d\n", distance(g, 1, 3, true));
        printf("d(3,4) = %d\n", distance(g, 3, 4, true));
        printf("diametro exato       = %d\n", diameter(g));
        printf("diametro aproximado  = %d\n", approximate_diameter(g));
        printf("\n");
        Graph_destroy(g);
    }
    return 0;
}
#endif
