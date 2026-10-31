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

/*
Todos os nos de um grafo em lista - as sentinelas graph[i] e os vizinhos -
moram num UNICO bloco que comeca em graph[0]: uma alocacao por grafo em vez
de uma por aresta. Nos grafos da Parte 2 (ate 50 milhoes de vertices e 73
milhoes de arestas), um malloc por no (24 bytes do no + 8 de cabecalho do
malloc) mais os vetores temporarios por vertice gastavam ~1,6x a memoria
do bloco (W_6: 6,1 GB contra 3,8 GB); no W_7 a leitura antiga enchia os
7,7 GB da maquina e passava a usar swap. Com o bloco, o no custa so os seus
24 bytes e o W_7 roda com 5,0 GB.
`vizinhos` e quantos nos de vizinho o bloco reserva depois das sentinelas.
*/
static Node **alocar_list_graph(int n, size_t vizinhos) {
    Node **graph = xmalloc((size_t)n * sizeof(Node *));
    if (n == 0) {
        return graph;
    }
    Node *bloco = xmalloc(((size_t)n + vizinhos) * sizeof(Node));
    for (int i = 0; i < n; i++) {
        bloco[i].value = -1; /* no Python: Node(None, None) */
        bloco[i].weight = 0.0;
        bloco[i].next = NULL;
        graph[i] = &bloco[i];
    }
    return graph;
}

Node **create_empty_list_graph(int n) {
    return alocar_list_graph(n, 0);
}

/* abre o arquivo e le a primeira linha (n); o leitor fica nas arestas */
static FILE *abrir_grafo(const char *archive_directory, Leitor *leitor, int *n) {
    FILE *file = fopen(archive_directory, "r");
    if (file == NULL) {
        fprintf(stderr, "arquivo nao encontrado: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }
    Leitor_init(leitor, file);
    if (!ler_inteiro(leitor, n) || *n < 0) {
        fprintf(stderr, "arquivo invalido: %s\n", archive_directory);
        exit(EXIT_FAILURE);
    }
    return file;
}

/*
Le o arquivo DUAS vezes, para nao guardar as arestas em memoria temporaria:
a 1a passada so conta o grau de cada vertice; com os graus, cada vertice
ganha o seu trecho contiguo no bloco de nos, e a 2a passada escreve cada
vizinho direto no lugar. Depois, cada trecho e ordenado e encadeado.
*/
Node **create_list_graph(const char *archive_directory, int *n_out,
                         bool *weighted_out, bool *negative_out) {
    int n = 0;
    Leitor *leitor = xmalloc(sizeof(Leitor)); /* 64 KB: fora da pilha */
    FILE *file = abrir_grafo(archive_directory, leitor, &n);

    int *grau = xcalloc((size_t)n + 1, sizeof(int));
    size_t total = 0; /* nos de vizinho: 2 por aresta (passa de 2^31) */
    bool weighted = false;
    bool negative = false;

    /* CORRECAO: o arquivo tem n vertices na 1a linha e depois m arestas,
       uma por linha. O laco antigo lia n linhas de aresta, o que so
       funciona quando m == n. Agora lemos ate o fim do arquivo. */
    int a = 0, b = 0;
    double peso = 1.0;
    while (ler_aresta(leitor, &a, &b, &peso)) {
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
        grau[a - 1] += 1;
        grau[b - 1] += 1;
        total += 2;
    }
    fclose(file);

    /* inicio[i]: onde comeca o trecho do vertice i no bloco de vizinhos */
    size_t *inicio = xmalloc(((size_t)n + 1) * sizeof(size_t));
    int maior_grau = 0;
    inicio[0] = 0;
    for (int i = 0; i < n; i++) {
        inicio[i + 1] = inicio[i] + (size_t)grau[i];
        if (grau[i] > maior_grau) {
            maior_grau = grau[i];
        }
        grau[i] = 0; /* vira o cursor de escrita da 2a passada */
    }

    Node **graph = alocar_list_graph(n, total);
    Node *vizinhos = (n > 0) ? graph[0] + n : NULL; /* depois das sentinelas */

    /* 2a passada: as mesmas arestas, na mesma ordem do arquivo */
    file = abrir_grafo(archive_directory, leitor, &n);
    while (ler_aresta(leitor, &a, &b, &peso)) {
        a -= 1;
        b -= 1;
        if (a == b) {
            continue;
        }
        Node *na = &vizinhos[inicio[a] + (size_t)grau[a]++];
        na->value = b;
        na->weight = peso;
        Node *nb = &vizinhos[inicio[b] + (size_t)grau[b]++];
        nb->value = a;
        nb->weight = peso;
    }
    fclose(file);
    free(leitor);

    /* no trecho de cada vertice, os vizinhos estao na ordem do arquivo:
       a posicao no trecho faz o papel da ordem de leitura no desempate */
    ArestaLida *trecho = xmalloc(((size_t)maior_grau + 1) * sizeof(ArestaLida));
    for (int i = 0; i < n; i++) {
        Node *base = &vizinhos[inicio[i]];
        for (int j = 0; j < grau[i]; j++) {
            trecho[j].vertice = base[j].value;
            trecho[j].peso = base[j].weight;
            trecho[j].ordem = j;
        }

        /* Vizinhos em ordem CRESCENTE: a BFS percorre a lista nessa ordem, a
           mesma da BFS_matrix (for 0..n-1), e as duas representacoes geram a
           mesma arvore. Com a lista decrescente, os empates de nivel eram
           resolvidos de forma diferente e os pais da BFS divergiam. */
        qsort(trecho, (size_t)grau[i], sizeof(ArestaLida), comparar_aresta);

        Node *ultimo = graph[i];
        int escritos = 0;
        for (int j = 0; j < grau[i]; j++) {
            /* aresta repetida no arquivo ("a b" duas vezes, ou "a b" e "b a"):
               ordenadas por (vertice, ordem), a copia fica logo ANTES da
               original e e descartada - vale a ultima linha, como na matriz
               (que sobrescreve o peso a cada linha repetida). */
            if (j + 1 < grau[i] && trecho[j + 1].vertice == trecho[j].vertice) {
                continue;
            }
            Node *no = &base[escritos++];
            no->value = trecho[j].vertice;
            no->weight = trecho[j].peso;
            no->next = NULL;
            ultimo->next = no;
            ultimo = no;
        }
    }

    free(trecho);
    free(inicio);
    free(grau);

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
    if (n > 0) {
        free(graph[0]); /* o bloco com todos os nos (ver alocar_list_graph) */
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
