#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph_gen.h"
#include "graph_utils.h"

/* aresta lida do arquivo, ainda sem vizinhos ordenados */
typedef struct {
    int vertice;
    double peso;
    int ordem; /* posicao no arquivo: entre duplicatas vale a ultima */
} ArestaLida;

/* vizinhos em ordem crescente de rotulo; entre iguais, a ordem do arquivo
   (para a duplicata certa, a ultima, prevalecer apos o qsort) */
static int comparar_aresta(const void *x, const void *y) {
    const ArestaLida *a = x;
    const ArestaLida *b = y;
    if (a->vertice != b->vertice) {
        return (a->vertice < b->vertice) ? -1 : 1;
    }
    return (a->ordem < b->ordem) ? -1 : 1;
}

/* Leitura rapida de numeros: os arquivos dos estudos de caso chegam a
   centenas de MB e o fscanf e lento demais. Le o arquivo em blocos e
   converte os numeros manualmente, com a mesma semantica do fscanf.
   A Parte 2 acrescentou a terceira coluna (peso, um double): ler_aresta
   le "a b" e, se houver um numero na MESMA linha, o peso. */

#define LEITOR_TAMANHO 65536

typedef struct {
    FILE *file;
    char buffer[LEITOR_TAMANHO];
    size_t lido;
    size_t posicao;
    int pushback; /* -2 = nenhum; senao, caractere devolvido ao leitor */
} Leitor;

static void Leitor_init(Leitor *leitor, FILE *file) {
    leitor->file = file;
    leitor->lido = 0;
    leitor->posicao = 0;
    leitor->pushback = -2;
}

static int ler_caractere(Leitor *leitor) {
    if (leitor->pushback != -2) {
        int c = leitor->pushback;
        leitor->pushback = -2;
        return c;
    }
    if (leitor->posicao >= leitor->lido) {
        leitor->lido = fread(leitor->buffer, 1, sizeof(leitor->buffer), leitor->file);
        leitor->posicao = 0;
        if (leitor->lido == 0) {
            return -1; /* fim do arquivo */
        }
    }
    return (unsigned char)leitor->buffer[leitor->posicao++];
}

/* devolve o ultimo caractere lido (um por vez): ler_aresta precisa dele
   para decidir, olhando o que vem depois de "a b", se ha peso na linha */
static void devolver_caractere(Leitor *leitor, int c) {
    leitor->pushback = c;
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
    devolver_caractere(leitor, c); /* o terminador volta para o proximo leitor */
    *valor = (int)(sinal * numero);
    return 1;
}

/* Devolve 1 se leu um numero em ponto flutuante, 0 se o token nao e numero.
   Aceita [-]digitos[.digitos][e[-]digitos] (e variantes como ".5" e "5."). */
static int ler_double(Leitor *leitor, double *valor) {
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
    if ((c < '0' || c > '9') && c != '.') {
        return 0; /* nao e numero */
    }

    long long inteiro = 0;
    while (c >= '0' && c <= '9') {
        inteiro = inteiro * 10 + (c - '0');
        c = ler_caractere(leitor);
    }

    double fracao = 0.0;
    double escala = 0.1;
    if (c == '.') {
        c = ler_caractere(leitor);
        while (c >= '0' && c <= '9') {
            fracao += (c - '0') * escala;
            escala *= 0.1;
            c = ler_caractere(leitor);
        }
    }

    int expoente = 0;
    if (c == 'e' || c == 'E') {
        c = ler_caractere(leitor);
        int esinal = 1;
        if (c == '-' || c == '+') {
            if (c == '-') {
                esinal = -1;
            }
            c = ler_caractere(leitor);
        }
        while (c >= '0' && c <= '9') {
            expoente = expoente * 10 + (c - '0');
            c = ler_caractere(leitor);
        }
        expoente *= esinal;
    }

    devolver_caractere(leitor, c); /* o terminador volta */

    *valor = sinal * ((double)inteiro + fracao);
    for (int i = 0; i < expoente; i++) {
        *valor *= 10.0;
    }
    for (int i = 0; i < -expoente; i++) {
        *valor /= 10.0;
    }
    return 1;
}

/*
Le uma aresta do arquivo (Parte 2): "a b" ou "a b peso". O peso pode ser
qualquer numero em ponto flutuante, negativo incluido; sem a terceira
coluna, a aresta vale 1. Devolve 0 no fim do arquivo (ou campo invalido).
*/
static int ler_aresta(Leitor *leitor, int *a, int *b, double *peso) {
    if (!ler_inteiro(leitor, a) || !ler_inteiro(leitor, b)) {
        return 0;
    }

    /* decide se ha peso na linha olhando o que vem depois de b: espacos
       seguidos de nova linha (ou fim do arquivo) = so duas colunas */
    int c = ler_caractere(leitor);
    while (c == ' ' || c == '\t') {
        c = ler_caractere(leitor);
    }
    if (c == '\n' || c == '\r' || c == -1) {
        *peso = 1.0; /* arquivo sem pesos: toda aresta vale 1 */
        return 1;
    }
    devolver_caractere(leitor, c);
    return ler_double(leitor, peso);
}

Node **create_empty_list_graph(int n) {
    Node **graph = xmalloc((size_t)n * sizeof(Node *));
    for (int i = 0; i < n; i++) {
        graph[i] = Node_create(-1, 0.0, NULL); /* no Python: Node(None, None) */
    }
    return graph;
}

/* expande neighbor[i], a lista dinamica que faz o papel do .append do Python */
static void neighbor_append(ArestaLida **neighbor, int *len, int *cap, int i,
                            int vertice, double peso, int ordem) {
    if (len[i] == cap[i]) {
        cap[i] = (cap[i] == 0) ? 4 : cap[i] * 2;
        neighbor[i] = xrealloc(neighbor[i], (size_t)cap[i] * sizeof(ArestaLida));
    }
    neighbor[i][len[i]].vertice = vertice;
    neighbor[i][len[i]].peso = peso;
    neighbor[i][len[i]].ordem = ordem;
    len[i] += 1;
}

Node **create_list_graph(const char *archive_directory, int *n_out,
                         bool *weighted_out, bool *negative_out) {
    FILE *file = fopen(archive_directory, "r");
    if (file == NULL) {
        fprintf(stderr, "arquivo nao encontrado: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }

    int n = 0;
    Leitor leitor;
    Leitor_init(&leitor, file);
    if (!ler_inteiro(&leitor, &n) || n < 0) {
        fprintf(stderr, "arquivo invalido: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }
    Node **graph = create_empty_list_graph(n);

    /* no Python: last_nodes = [None for _ in range(n)]; neighbor = {i: []} */
    Node **last_nodes = xcalloc((size_t)n, sizeof(Node *));
    ArestaLida **neighbor = xcalloc((size_t)n, sizeof(ArestaLida *));
    int *neighbor_len = xcalloc((size_t)n, sizeof(int));
    int *neighbor_cap = xcalloc((size_t)n, sizeof(int));

    bool weighted = false;
    bool negative = false;
    int ordem = 0;

    /* CORRECAO: o arquivo tem n vertices na 1a linha e depois m arestas,
       uma por linha. O laco antigo lia n linhas de aresta, o que so
       funciona quando m == n. Agora lemos ate o fim do arquivo. */
    int a = 0, b = 0;
    double peso = 1.0;
    while (ler_aresta(&leitor, &a, &b, &peso)) {
        a -= 1;
        b -= 1;

        /* laco ("v v"): ignorado. O grafo e tratado como simples - antes o
           laco somava 2 ao grau na lista e 1 na matriz, e as representacoes
           reportavam graus e numeros de arestas diferentes. */
        if (a == b) {
            continue;
        }

        if (peso != 1.0) {
            weighted = true;
        }
        if (peso < 0.0) {
            negative = true;
        }

        neighbor_append(neighbor, neighbor_len, neighbor_cap, a, b, peso, ordem);
        ordem += 1;
        neighbor_append(neighbor, neighbor_len, neighbor_cap, b, a, peso, ordem);
        ordem += 1;
    }
    fclose(file);

    for (int i = 0; i < n; i++) {
        /* Vizinhos em ordem CRESCENTE: a BFS percorre a lista nessa ordem, a
           mesma da BFS_matrix (for 0..n-1), e as duas representacoes geram a
           mesma arvore. Com a lista decrescente, os empates de nivel eram
           resolvidos de forma diferente e os pais da BFS divergiam. */
        qsort(neighbor[i], (size_t)neighbor_len[i], sizeof(ArestaLida), comparar_aresta);

        for (int j = 0; j < neighbor_len[i]; j++) {
            /* aresta repetida no arquivo ("a b" duas vezes, ou "a b" e "b a"):
               ordenadas por (vertice, ordem), a copia fica logo ANTES da
               original e e descartada - vale a ultima linha, como na matriz
               (que sobrescreve o peso a cada linha repetida). */
            if (j + 1 < neighbor_len[i] && neighbor[i][j + 1].vertice == neighbor[i][j].vertice) {
                continue;
            }
            if (last_nodes[i] == NULL) {
                graph[i]->next = Node_create(neighbor[i][j].vertice, neighbor[i][j].peso, NULL);
                last_nodes[i] = graph[i]->next;
            } else {
                last_nodes[i]->next = Node_create(neighbor[i][j].vertice, neighbor[i][j].peso, NULL);
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
    *weighted_out = weighted;
    *negative_out = negative;
    return graph;
}

MatrixCell **create_empty_matrix_graph(int n) {
    MatrixCell **graph = xmalloc((size_t)n * sizeof(MatrixCell *));
    for (int i = 0; i < n; i++) {
        graph[i] = xcalloc((size_t)n, sizeof(MatrixCell));
    }
    return graph;
}

/* matriz de pesos: n x n doubles zerados (a adjacencia disambigua peso 0 de
   "sem aresta": o peso so e lido para celulas com aresta) */
static double **criar_matrix_weights(int n) {
    double **weights = xmalloc((size_t)n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        weights[i] = xcalloc((size_t)n, sizeof(double));
    }
    return weights;
}

MatrixCell **create_matrix_graph(const char *archive_directory, int *n_out,
                                 double ***weights_out, bool *weighted_out,
                                 bool *negative_out) {
    FILE *file = fopen(archive_directory, "r");
    if (file == NULL) {
        fprintf(stderr, "arquivo nao encontrado: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }

    int n = 0;
    Leitor leitor;
    Leitor_init(&leitor, file);
    if (!ler_inteiro(&leitor, &n) || n < 0) {
        fprintf(stderr, "arquivo invalido: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }
    MatrixCell **graph = create_empty_matrix_graph(n);

    /* A matriz de pesos so e criada se o arquivo tem algum peso != 1: n^2
       doubles seriam um desperdicio enorme nos grafos da Parte 1 (a matriz
       de 1 byte ja e o suficiente para eles). Como ela nasce na PRIMEIRA
       aresta com peso != 1, as arestas lidas antes dela ganham 1.0 aqui. */
    double **weights = NULL;
    bool weighted = false;
    bool negative = false;

    /* CORRECAO: o arquivo tem n vertices na 1a linha e depois m arestas,
       uma por linha. O laco antigo lia n linhas de aresta, o que so
       funciona quando m == n. Agora lemos ate o fim do arquivo. */
    int a = 0, b = 0;
    double peso = 1.0;
    while (ler_aresta(&leitor, &a, &b, &peso)) {
        a -= 1;
        b -= 1;

        /* laco ignorado, como na lista de adjacencia (grafo simples) */
        if (a == b) {
            continue;
        }

        if (peso != 1.0 && !weighted) {
            weighted = true;
            weights = criar_matrix_weights(n);
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (graph[i][j] != 0) {
                        weights[i][j] = 1.0; /* arestas lidas antes tinham peso 1 */
                    }
                }
            }
        }
        if (peso < 0.0) {
            negative = true;
        }

        /* entre linhas repetidas da mesma aresta, a ultima prevalece */
        graph[a][b] = 1;
        graph[b][a] = 1;
        if (weights != NULL) {
            weights[a][b] = peso;
            weights[b][a] = peso;
        }
    }
    fclose(file);

    *n_out = n;
    *weights_out = weights;
    *weighted_out = weighted;
    *negative_out = negative;
    return graph;
}

char *str_adj_list(Node **graph, int n) {
    StrBuf result = {NULL, 0, 0};
    for (int i = 0; i < n; i++) {
        strbuf_appendf(&result, "%d: ", i);
        Node *current_node = graph[i]->next;
        while (current_node != NULL) {
            strbuf_appendf(&result, "%d -> ", current_node->value);
            current_node = current_node->next;
        }
        strbuf_appendf(&result, "None\n");
    }
    return strbuf_finish(&result);
}

char *str_matrix(MatrixCell **graph, int n) {
    StrBuf result = {NULL, 0, 0};
    for (int i = 0; i < n; i++) {
        strbuf_appendf(&result, "[");
        for (int j = 0; j < n; j++) {
            strbuf_appendf(&result, (j == 0) ? "%d" : ", %d", graph[i][j]);
        }
        strbuf_appendf(&result, "]\n");
    }
    return strbuf_finish(&result);
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

void free_matrix_graph(MatrixCell **graph, int n) {
    for (int i = 0; i < n; i++) {
        free(graph[i]);
    }
    free(graph);
}

void free_matrix_weights(double **weights, int n) {
    if (weights == NULL) {
        return;
    }
    for (int i = 0; i < n; i++) {
        free(weights[i]);
    }
    free(weights);
}
