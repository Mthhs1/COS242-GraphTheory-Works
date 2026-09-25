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

void strbuf_appendf(StrBuf *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int needed = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (needed < 0) {
        return;
    }

    size_t required = buffer->len + (size_t)needed + 1;
    if (required > buffer->cap) {
        size_t cap = (buffer->cap == 0) ? 256 : buffer->cap;
        while (cap < required) {
            cap *= 2;
        }
        buffer->data = xrealloc(buffer->data, cap);
        buffer->cap = cap;
    }

    va_start(args, format);
    vsnprintf(buffer->data + buffer->len, (size_t)needed + 1, format, args);
    va_end(args);
    buffer->len += (size_t)needed;
}

char *strbuf_finish(StrBuf *buffer) {
    if (buffer->data == NULL) {
        buffer->data = xmalloc(1);
        buffer->data[0] = '\0';
    }
    char *text = buffer->data;
    buffer->data = NULL;
    buffer->len = 0;
    buffer->cap = 0;
    return text;
}

int degree(Graph *g, int v) {
    int d = 0;
    int vizinho;
    NeighborIter it = neighbor_iter(g, v);
    while (neighbor_next(&it, &vizinho)) {
        d += 1;
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
