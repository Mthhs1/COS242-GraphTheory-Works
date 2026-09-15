#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph_gen.h"
#include "graph_utils.h"

static int comparar_descendente(const void *x, const void *y) {
    int a = *(const int *)x;
    int b = *(const int *)y;
    if (a > b) return -1;
    if (a < b) return 1;
    return 0;
}

/* Leitura rapida de inteiros: os arquivos dos estudos de caso chegam a
   centenas de MB e o fscanf e lento demais. Le o arquivo em blocos e
   converte os numeros manualmente, com a mesma semantica do fscanf %d. */

#define LEITOR_TAMANHO 65536

typedef struct {
    FILE *file;
    char buffer[LEITOR_TAMANHO];
    size_t lido;
    size_t posicao;
} Leitor;

static int ler_caractere(Leitor *leitor) {
    if (leitor->posicao >= leitor->lido) {
        leitor->lido = fread(leitor->buffer, 1, sizeof(leitor->buffer), leitor->file);
        leitor->posicao = 0;
        if (leitor->lido == 0) {
            return -1; /* fim do arquivo */
        }
    }
    return (unsigned char)leitor->buffer[leitor->posicao++];
}

/* Devolve 1 se leu um inteiro, 0 no fim do arquivo (ou caractere invalido). */
static int ler_inteiro(Leitor *leitor, int *valor) {
    int c = ler_caractere(leitor);
    while (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
        c = ler_caractere(leitor);
    }
    if (c == -1) {
        return 0;
    }

    int sinal = 1;
    if (c == '-' || c == '+') {
        if (c == '-') {
            sinal = -1;
        }
        c = ler_caractere(leitor);
    }
    if (c < '0' || c > '9') {
        return 0; /* caractere inesperado: para a leitura */
    }

    long numero = 0;
    while (c >= '0' && c <= '9') {
        numero = numero * 10 + (c - '0');
        c = ler_caractere(leitor);
    }
    *valor = (int)(sinal * numero);
    return 1;
}

Node **create_empty_list_graph(int n) {
    Node **graph = xmalloc((size_t)n * sizeof(Node *));
    for (int i = 0; i < n; i++) {
        graph[i] = Node_create(-1, NULL); /* no Python: Node(None, None) */
    }
    return graph;
}

/* expande neighbor[i], a lista dinamica que faz o papel do .append do Python */
static void neighbor_append(int **neighbor, int *len, int *cap, int i, int value) {
    if (len[i] == cap[i]) {
        cap[i] = (cap[i] == 0) ? 4 : cap[i] * 2;
        neighbor[i] = xrealloc(neighbor[i], (size_t)cap[i] * sizeof(int));
    }
    neighbor[i][len[i]] = value;
    len[i] += 1;
}

Node **create_list_graph(const char *archive_directory, int *n_out) {
    FILE *file = fopen(archive_directory, "r");
    if (file == NULL) {
        fprintf(stderr, "arquivo nao encontrado: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }

    int n = 0;
    Leitor leitor;
    leitor.file = file;
    leitor.lido = 0;
    leitor.posicao = 0;
    if (!ler_inteiro(&leitor, &n) || n < 0) {
        fprintf(stderr, "arquivo invalido: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }
    Node **graph = create_empty_list_graph(n);

    /* no Python: last_nodes = [None for _ in range(n)]; neighbor = {i: []} */
    Node **last_nodes = xcalloc((size_t)n, sizeof(Node *));
    int **neighbor = xcalloc((size_t)n, sizeof(int *));
    int *neighbor_len = xcalloc((size_t)n, sizeof(int));
    int *neighbor_cap = xcalloc((size_t)n, sizeof(int));

    /* CORRECAO: o arquivo tem n vertices na 1a linha e depois m arestas,
       uma por linha. O laco antigo lia n linhas de aresta, o que so
       funciona quando m == n. Agora lemos ate o fim do arquivo. */
    int a = 0, b = 0;
    while (ler_inteiro(&leitor, &a) && ler_inteiro(&leitor, &b)) {
        a -= 1;
        b -= 1;

        neighbor_append(neighbor, neighbor_len, neighbor_cap, a, b);
        neighbor_append(neighbor, neighbor_len, neighbor_cap, b, a);
    }
    fclose(file);

    for (int i = 0; i < n; i++) {
        /* neighbor[i].sort(reverse=True) */
        qsort(neighbor[i], (size_t)neighbor_len[i], sizeof(int), comparar_descendente);

        for (int j = 0; j < neighbor_len[i]; j++) {
            if (last_nodes[i] == NULL) {
                graph[i]->next = Node_create(neighbor[i][j], NULL);
                last_nodes[i] = graph[i]->next;
            } else {
                last_nodes[i]->next = Node_create(neighbor[i][j], NULL);
                last_nodes[i] = last_nodes[i]->next;
            }
        }
        free(neighbor[i]);
    }

    free(neighbor);
    free(neighbor_len);
    free(neighbor_cap);
    free(last_nodes);

    *n_out = n;
    return graph;
}

int **create_empty_matrix_graph(int n) {
    int **graph = xmalloc((size_t)n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        graph[i] = xcalloc((size_t)n, sizeof(int));
    }
    return graph;
}

int **create_matrix_graph(const char *archive_directory, int *n_out) {
    FILE *file = fopen(archive_directory, "r");
    if (file == NULL) {
        fprintf(stderr, "arquivo nao encontrado: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }

    int n = 0;
    Leitor leitor;
    leitor.file = file;
    leitor.lido = 0;
    leitor.posicao = 0;
    if (!ler_inteiro(&leitor, &n) || n < 0) {
        fprintf(stderr, "arquivo invalido: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }
    int **graph = create_empty_matrix_graph(n);

    /* CORRECAO: o arquivo tem n vertices na 1a linha e depois m arestas,
       uma por linha. O laco antigo lia n linhas de aresta, o que so
       funciona quando m == n. Agora lemos ate o fim do arquivo. */
    int a = 0, b = 0;
    while (ler_inteiro(&leitor, &a) && ler_inteiro(&leitor, &b)) {
        a -= 1;
        b -= 1;

        graph[a][b] = 1;
        graph[b][a] = 1;
    }
    fclose(file);

    *n_out = n;
    return graph;
}

char *str_adj_list(Node **graph, int n) {
    char *result = NULL;
    for (int i = 0; i < n; i++) {
        result = str_append(result, "%d: ", i);
        Node *current_node = graph[i]->next;
        while (current_node != NULL) {
            result = str_append(result, "%d -> ", current_node->value);
            current_node = current_node->next;
        }
        result = str_append(result, "None\n");
    }
    return result;
}

char *str_matrix(int **graph, int n) {
    char *result = NULL;
    for (int i = 0; i < n; i++) {
        result = str_append(result, "[");
        for (int j = 0; j < n; j++) {
            result = str_append(result, (j == 0) ? "%d" : ", %d", graph[i][j]);
        }
        result = str_append(result, "]\n");
    }
    return result;
}

void free_list_graph(Node **graph, int n) {
    for (int i = 0; i < n; i++) {
        Node *current = graph[i]->next;
        while (current != NULL) {
            Node *next = current->next;
            free(current);
            current = next;
        }
        free(graph[i]); /* o no cabecalho */
    }
    free(graph);
}

void free_matrix_graph(int **graph, int n) {
    for (int i = 0; i < n; i++) {
        free(graph[i]);
    }
    free(graph);
}
