#include <stdlib.h>
#include <string.h>

#include "components.h"
#include "my_queue.h"
#include "graph_utils.h"

static int comparar_ascendente(const void *x, const void *y) {
    int a = *(const int *)x;
    int b = *(const int *)y;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/*
Retorna a lista de componentes conexas, cada uma como lista de rotulos
1-based ordenados, e o conjunto todo ordenado por tamanho decrescente.
*/

Components connected_components(Graph *g) {
    int n = g->n;
    bool *visited = xcalloc((size_t)n, sizeof(bool));

    Components result;
    result.count = 0;
    result.components = NULL;
    result.sizes = NULL;
    int cap = 0;

    for (int source = 0; source < n; source++) {
        if (visited[source]) {
            continue;
        }

        /* BFS a partir de cada vertice ainda nao visitado: cada varredura
           esgota exatamente uma componente. O vetor cresce conforme a BFS
           encontra vertices: alocar n inteiros por componente desperdiciaria
           memoria em grafos com muitas componentes. */
        int component_cap = 16;
        int *component = xmalloc((size_t)component_cap * sizeof(int));
        int component_len = 0;

        Queue queue;
        Queue_init(&queue);
        Queue_enqueue(&queue, source);
        visited[source] = true;

        while (!Queue_isEmpty(&queue)) {
            int current = Queue_dequeue(&queue);

            if (component_len == component_cap) {
                component_cap *= 2;
                component = xrealloc(component, (size_t)component_cap * sizeof(int));
            }
            component[component_len] = current + 1; /* rotulo 1-based, como no arquivo */
            component_len += 1;

            int neighbor_count;
            int *neighbor = neighbors(g, current, &neighbor_count);
            for (int j = 0; j < neighbor_count; j++) {
                if (!visited[neighbor[j]]) {
                    visited[neighbor[j]] = true;
                    Queue_enqueue(&queue, neighbor[j]);
                }
            }
            free(neighbor);
        }

        /* component.sort() */
        qsort(component, (size_t)component_len, sizeof(int), comparar_ascendente);

        /* components.append(component) */
        if (result.count == cap) {
            cap = (cap == 0) ? 4 : cap * 2;
            result.components = xrealloc(result.components, (size_t)cap * sizeof(int *));
            result.sizes = xrealloc(result.sizes, (size_t)cap * sizeof(int));
        }
        result.components[result.count] = component;
        result.sizes[result.count] = component_len;
        result.count += 1;
    }

    /* components.sort(key=len, reverse=True) do Python: insertion sort,
       que como o sort do Python e estavel (empates mantem a ordem de
       descoberta) */
    for (int i = 1; i < result.count; i++) {
        int *component = result.components[i];
        int size = result.sizes[i];
        int j = i - 1;
        while (j >= 0 && result.sizes[j] < size) {
            result.components[j + 1] = result.components[j];
            result.sizes[j + 1] = result.sizes[j];
            j -= 1;
        }
        result.components[j + 1] = component;
        result.sizes[j + 1] = size;
    }

    free(visited);
    return result;
}

void component_of(Graph *g, Components *result, int **component_id) {
    /*
    Vetor auxiliar: component_id[v] diz a qual componente (indice na lista
    devolvida por connected_components) o vertice 0-based v pertence.
    Util para o diametro, que so faz sentido dentro de uma componente.
    */
    Components components = connected_components(g);
    int *cid = xmalloc((size_t)g->n * sizeof(int));
    for (int v = 0; v < g->n; v++) {
        cid[v] = -1;
    }

    for (int index = 0; index < components.count; index++) {
        for (int j = 0; j < components.sizes[index]; j++) {
            int label = components.components[index][j];
            cid[label - 1] = index;
        }
    }

    *result = components;
    *component_id = cid;
}

char *summary(Graph *g) {
    /* Resumo textual das componentes, no formato usado no arquivo de saida. */
    Components components = connected_components(g);

    /* um append por vertice: StrBuf mantem o custo linear */
    StrBuf text = {NULL, 0, 0};
    strbuf_appendf(&text, "Numero de componentes conexas: %d", components.count);

    if (components.count > 0) {
        strbuf_appendf(&text, "\nMaior componente: %d vertices", components.sizes[0]);
        strbuf_appendf(&text, "\nMenor componente: %d vertices",
                       components.sizes[components.count - 1]);
    }

    strbuf_appendf(&text, "\n");
    for (int index = 0; index < components.count; index++) {
        strbuf_appendf(&text, "\nComponente %d (%d vertices):", index + 1,
                       components.sizes[index]);
        strbuf_appendf(&text, "\n  ");
        for (int j = 0; j < components.sizes[index]; j++) {
            strbuf_appendf(&text, (j == 0) ? "%d" : " %d",
                           components.components[index][j]);
        }
    }

    Components_free(&components);
    return strbuf_finish(&text);
}

void Components_free(Components *components) {
    for (int i = 0; i < components->count; i++) {
        free(components->components[i]);
    }
    free(components->components);
    free(components->sizes);
    components->count = 0;
    components->components = NULL;
    components->sizes = NULL;
}

#ifdef COMPONENTS_MAIN
/* if __name__ == "__main__" do components.py */
#include <stdio.h>

int main(void) {
    for (int use_list = 0; use_list <= 1; use_list++) {
        const char *rep = use_list ? "lista de adjacencia" : "matriz de adjacencia";
        Graph *g = Graph_create("graph_exemplo.txt", use_list != 0);
        printf("--- %s ---\n", rep);
        char *text = summary(g);
        printf("%s\n\n", text);
        free(text);
        Graph_destroy(g);
    }
    return 0;
}
#endif
