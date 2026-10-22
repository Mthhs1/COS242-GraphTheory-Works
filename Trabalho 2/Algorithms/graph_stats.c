#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph_stats.h"
#include "graph_utils.h"
#include "components.h"

static int comparar_ascendente(const void *x, const void *y) {
    int a = *(const int *)x;
    int b = *(const int *)y;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

double median(int *values, int len) {
    /* Mediana de uma lista de numeros (media dos dois centrais se par). */
    if (len == 0) {
        return 0.0;
    }

    int *ordered = xmalloc((size_t)len * sizeof(int));
    for (int i = 0; i < len; i++) {
        ordered[i] = values[i];
    }
    qsort(ordered, (size_t)len, sizeof(int), comparar_ascendente);

    int middle = len / 2;

    double result;
    if (len % 2 == 1) {
        result = (double)ordered[middle];
    } else {
        result = (ordered[middle - 1] + ordered[middle]) / 2.0;
    }

    free(ordered);
    return result;
}

GraphStats graph_stats(Graph *g) {
    /* Dicionario com as estatisticas exigidas no requisito 2. */
    int *degrees = all_degrees(g);
    int n = g->n;

    GraphStats s;
    s.vertices = n;
    s.arestas = edge_count(g);
    s.grau_minimo = 0;
    s.grau_maximo = 0;
    s.grau_medio = 0.0;

    if (n > 0) {
        s.grau_minimo = degrees[0];
        s.grau_maximo = degrees[0];
        int soma = 0;
        for (int v = 0; v < n; v++) {
            if (degrees[v] < s.grau_minimo) {
                s.grau_minimo = degrees[v];
            }
            if (degrees[v] > s.grau_maximo) {
                s.grau_maximo = degrees[v];
            }
            soma += degrees[v];
        }
        s.grau_medio = (double)soma / n;
    }

    s.mediana_grau = median(degrees, n);

    free(degrees);
    return s;
}

char *stats_text(Graph *g) {
    /* Bloco de texto com as estatisticas, no formato do arquivo de saida. */
    GraphStats s = graph_stats(g);

    char *text = str_append(NULL, "Numero de vertices: %d", s.vertices);
    text = str_append(text, "\nNumero de arestas: %d", s.arestas);
    text = str_append(text, "\nGrau minimo: %d", s.grau_minimo);
    text = str_append(text, "\nGrau maximo: %d", s.grau_maximo);
    text = str_append(text, "\nGrau medio: %.4f", s.grau_medio);
    text = str_append(text, "\nMediana de grau: %.1f", s.mediana_grau);
    return text;
}

char *write_output(Graph *g, const char *output_path) {
    /* Escreve o arquivo de saida completo exigido pelo requisito 2. */
    char *stats = stats_text(g);
    char *comp = summary(g);

    char *content = str_append(NULL,
                              "=== INFORMACOES DO GRAFO ===\n\n%s\n\n"
                              "=== COMPONENTES CONEXAS ===\n\n%s\n",
                              stats, comp);
    free(stats);
    free(comp);

    FILE *file = fopen(output_path, "w");
    if (file == NULL) {
        fprintf(stderr, "nao foi possivel escrever: %s\n", output_path);
        exit(EXIT_FAILURE);
    }
    fputs(content, file);
    fclose(file);

    return content;
}

char *write_search_tree(int *parents, int *levels, int n, const char *output_path, int root) {
    /*
    Requisito 4: grava a arvore de busca (pai e nivel de cada vertice).
    `parents` e `levels` sao os vetores devolvidos por BFS/DFS com
    generate_tree, indexados em 0.
    */
    StrBuf buffer = {NULL, 0, 0}; /* uma linha por vertice: custo linear */
    strbuf_appendf(&buffer, "Raiz da busca: %d\n\nvertice pai nivel\n", root + 1);

    for (int vertex = 0; vertex < n; vertex++) {
        if (levels[vertex] == -1 && vertex != root) {
            continue; /* vertice fora da componente da raiz */
        }

        int parent = parents[vertex];
        if (parent == -1) { /* None no Python */
            strbuf_appendf(&buffer, "%d - %d\n", vertex + 1, levels[vertex]);
        } else {
            strbuf_appendf(&buffer, "%d %d %d\n", vertex + 1, parent + 1, levels[vertex]);
        }
    }
    char *content = strbuf_finish(&buffer);

    FILE *file = fopen(output_path, "w");
    if (file == NULL) {
        fprintf(stderr, "nao foi possivel escrever: %s\n", output_path);
        exit(EXIT_FAILURE);
    }
    fputs(content, file);
    fclose(file);

    return content;
}

#ifdef GRAPH_STATS_MAIN
/* if __name__ == "__main__" do graph_stats.py */
int main(void) {
    Graph *g = Graph_create("graph_exemplo.txt", true);
    char *content = write_output(g, "saida_exemplo.txt");
    printf("%s", content);
    free(content);
    Graph_destroy(g);
    return 0;
}
#endif
