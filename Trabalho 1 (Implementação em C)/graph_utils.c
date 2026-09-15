#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph_utils.h"

void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "falha ao alocar memoria\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *xcalloc(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    if (ptr == NULL) {
        fprintf(stderr, "falha ao alocar memoria\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    void *novo = realloc(ptr, size);
    if (novo == NULL) {
        fprintf(stderr, "falha ao alocar memoria\n");
        exit(EXIT_FAILURE);
    }
    return novo;
}

char *str_append(char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int needed = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (needed < 0) {
        return buffer;
    }

    size_t old_len = (buffer == NULL) ? 0 : strlen(buffer);
    buffer = xrealloc(buffer, old_len + (size_t)needed + 1);

    va_start(args, format);
    vsnprintf(buffer + old_len, (size_t)needed + 1, format, args);
    va_end(args);
    return buffer;
}

int *neighbors(Graph *g, int v, int *count_out) {
    int count = 0;

    if (g->adj_list) {
        Node *node = g->graph.list[v]->next;
        while (node != NULL) {
            count += 1;
            node = node->next;
        }

        int *vizinhos = xmalloc((size_t)(count > 0 ? count : 1) * sizeof(int));
        int k = 0;
        node = g->graph.list[v]->next;
        while (node != NULL) {
            vizinhos[k] = node->value;
            k += 1;
            node = node->next;
        }

        *count_out = count;
        return vizinhos;
    }

    int *row = g->graph.matrix[v];
    for (int u = 0; u < g->n; u++) {
        if (row[u] == 1) {
            count += 1;
        }
    }

    int *vizinhos = xmalloc((size_t)(count > 0 ? count : 1) * sizeof(int));
    int k = 0;
    for (int u = 0; u < g->n; u++) {
        if (row[u] == 1) {
            vizinhos[k] = u;
            k += 1;
        }
    }

    *count_out = count;
    return vizinhos;
}

int degree(Graph *g, int v) {
    if (g->adj_list) {
        int d = 0;
        Node *node = g->graph.list[v]->next;
        while (node != NULL) {
            d += 1;
            node = node->next;
        }
        return d;
    }

    int d = 0;
    for (int u = 0; u < g->n; u++) {
        d += g->graph.matrix[v][u];
    }
    return d;
}

int *all_degrees(Graph *g) {
    int *degrees = xmalloc((size_t)g->n * sizeof(int));
    for (int v = 0; v < g->n; v++) {
        degrees[v] = degree(g, v);
    }
    return degrees;
}

int edge_count(Graph *g) {
    int *degrees = all_degrees(g);

    int soma = 0;
    for (int v = 0; v < g->n; v++) {
        soma += degrees[v];
    }
    free(degrees);

    return soma / 2; /* lema do aperto de mao: m = (1/2) * soma dos graus */
}
