/*
Questao da "rede de colaboracao" da Parte 2 (estudos de caso): distancia e
caminho minimo entre Edsger W. Dijkstra e Alan M. Turing, J. B. Kruskal,
Jon M. Kleinberg, Eva Tardos e Daniel R. Figueiredo (a grafia exata das
strings e a do enunciado).

uso: ./bin/p2_questao3_colaboracao <grafo.txt> <nomes.txt> [vetor|heap]

Formato do arquivo de nomes do site da disciplina: a linha i (1-based) e
"i,nome" - o rotulo do vertice, uma virgula e o nome (ex.: "2722,Edsger W.
Dijkstra"). nome_da_linha descarta o prefixo "i,"; uma linha sem o prefixo
numerico vale inteira como nome (o formato suposto antes do arquivo sair).

Duas passadas pelo arquivo de nomes (uma para achar os rotulos dos
pesquisadores, outra para imprimir os caminhos com nomes): a rede tem
milhoes de vertices e guardar todos os nomes nao compensa.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graph.h"
#include "graph_utils.h"
#include "dijkstra.h"

#define TAM_LINHA 512

static const char *FONTE = "Edsger W. Dijkstra";
static const char *ALVOS[] = {
    "Alan M. Turing",
    "J. B. Kruskal",
    "Jon M. Kleinberg",
    "Éva Tardos",
    "Daniel R. Figueiredo",
};
#define N_ALVOS ((int)(sizeof(ALVOS) / sizeof(ALVOS[0])))

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <nomes.txt> [vetor|heap]\n", programa);
    exit(EXIT_FAILURE);
}

/* tira \n e \r do fim da linha lida com fgets */
static void tirar_fim_de_linha(char *linha) {
    size_t fim = strlen(linha);
    while (fim > 0 && (linha[fim - 1] == '\n' || linha[fim - 1] == '\r')) {
        fim -= 1;
    }
    linha[fim] = '\0';
}

/* "2722,Edsger W. Dijkstra" -> "Edsger W. Dijkstra": pula o rotulo e a virgula */
static const char *nome_da_linha(const char *linha) {
    const char *c = linha;
    while (*c >= '0' && *c <= '9') {
        c += 1;
    }
    if (c != linha && *c == ',') {
        return c + 1;
    }
    return linha;
}

/*
Passada 1: acha o rotulo (1-based) dos pesquisadores no arquivo de nomes.
Devolve false se algum nao foi encontrado (e imprime quem falta).
*/
static bool encontrar_rotulos(const char *caminho_nomes, int *rotulo_fonte, int *rotulos_alvos) {
    FILE *file = fopen(caminho_nomes, "r");
    if (file == NULL) {
        fprintf(stderr, "arquivo nao encontrado: %s\n", caminho_nomes);
        return false;
    }

    *rotulo_fonte = 0;
    for (int i = 0; i < N_ALVOS; i++) {
        rotulos_alvos[i] = 0;
    }

    char linha[TAM_LINHA];
    int rotulo = 0;
    while (fgets(linha, sizeof linha, file) != NULL) {
        rotulo += 1;
        tirar_fim_de_linha(linha);
        const char *nome = nome_da_linha(linha);

        if (*rotulo_fonte == 0 && strcmp(nome, FONTE) == 0) {
            *rotulo_fonte = rotulo;
        }
        for (int i = 0; i < N_ALVOS; i++) {
            if (rotulos_alvos[i] == 0 && strcmp(nome, ALVOS[i]) == 0) {
                rotulos_alvos[i] = rotulo;
            }
        }
        if (*rotulo_fonte != 0) {
            bool todos = true;
            for (int i = 0; i < N_ALVOS; i++) {
                if (rotulos_alvos[i] == 0) {
                    todos = false;
                }
            }
            if (todos) {
                break;
            }
        }
    }
    fclose(file);

    bool ok = true;
    if (*rotulo_fonte == 0) {
        fprintf(stderr, "nao esta no arquivo de nomes: \"%s\"\n", FONTE);
        ok = false;
    }
    for (int i = 0; i < N_ALVOS; i++) {
        if (rotulos_alvos[i] == 0) {
            fprintf(stderr, "nao esta no arquivo de nomes: \"%s\"\n", ALVOS[i]);
            ok = false;
        }
    }
    return ok;
}

/*
Passada 2: carrega os nomes dos vertices que precisam ser impressos
(marcados em preciso[vertice]) e guarda strdup deles em nomes[vertice].
*/
static void carregar_nomes_precisos(const char *caminho_nomes, int n, const bool *preciso,
                                    char **nomes) {
    FILE *file = fopen(caminho_nomes, "r");
    if (file == NULL) {
        return; /* as impressoes caem no rotulo numerico */
    }

    char linha[TAM_LINHA];
    int rotulo = 0;
    while (fgets(linha, sizeof linha, file) != NULL && rotulo < n) {
        int indice = rotulo; /* rotulo 1-based -> indice 0-based */
        rotulo += 1;
        if (!preciso[indice]) {
            continue;
        }
        tirar_fim_de_linha(linha);
        const char *nome = nome_da_linha(linha);
        nomes[indice] = xmalloc(strlen(nome) + 1);
        strcpy(nomes[indice], nome);
    }
    fclose(file);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        uso(argv[0]);
    }

    DijkstraModo modo = DIJKSTRA_HEAP;
    if (argc >= 4) {
        if (strcmp(argv[3], "vetor") == 0) {
            modo = DIJKSTRA_VETOR;
        } else if (strcmp(argv[3], "heap") == 0) {
            modo = DIJKSTRA_HEAP;
        } else {
            fprintf(stderr, "modo invalido: %s (use vetor ou heap)\n", argv[3]);
            return EXIT_FAILURE;
        }
    }

    int rotulo_fonte;
    int rotulos_alvos[N_ALVOS];
    if (!encontrar_rotulos(argv[2], &rotulo_fonte, rotulos_alvos)) {
        return EXIT_FAILURE;
    }

    /* a rede de colaboracao e grande: representacao em lista */
    Graph *g = Graph_create(argv[1], true);

    if (rotulo_fonte > g->n) {
        fprintf(stderr, "rotulo da fonte (%d) maior que o grafo (%d vertices)\n",
                rotulo_fonte, g->n);
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    printf("grafo: %s | nomes: %s | modo: %s | %d vertices\n",
           argv[1], argv[2], modo == DIJKSTRA_HEAP ? "heap" : "vetor", g->n);
    printf("vertice de \"%s\": %d\n", FONTE, rotulo_fonte);

    DijkstraResult *r = dijkstra(g, rotulo_fonte - 1, modo);
    if (r == NULL) {
        Graph_destroy(g);
        return EXIT_FAILURE;
    }

    /* marca os vertices dos caminhos para carregar so esses nomes */
    bool *preciso = xcalloc((size_t)g->n, sizeof(bool));
    preciso[rotulo_fonte - 1] = true;
    int *caminhos[N_ALVOS];
    int comprimentos[N_ALVOS];
    for (int i = 0; i < N_ALVOS; i++) {
        caminhos[i] = NULL;
        comprimentos[i] = 0;
        if (r->dist[rotulos_alvos[i] - 1] != INFINITO_PESO) {
            caminhos[i] = dijkstra_path(r, rotulos_alvos[i] - 1, &comprimentos[i]);
            for (int j = 0; j < comprimentos[i]; j++) {
                preciso[caminhos[i][j]] = true;
            }
        }
    }

    char **nomes = xcalloc((size_t)g->n, sizeof(char *));
    carregar_nomes_precisos(argv[2], g->n, preciso, nomes);

    for (int i = 0; i < N_ALVOS; i++) {
        int alvo = rotulos_alvos[i];
        printf("\n%s (vertice %d)\n", ALVOS[i], alvo);

        if (caminhos[i] == NULL) {
            printf("  distancia: infinita (sem caminho)\n");
            continue;
        }

        printf("  distancia: %g\n", r->dist[alvo - 1]);
        printf("  caminho:");
        for (int j = 0; j < comprimentos[i]; j++) {
            int v = caminhos[i][j];
            if (nomes[v] != NULL) {
                printf("%s%s", (j == 0) ? " " : " -> ", nomes[v]);
            } else {
                printf("%s%d", (j == 0) ? " " : " -> ", v + 1);
            }
        }
        printf("\n");
        free(caminhos[i]);
    }

    for (int v = 0; v < g->n; v++) {
        free(nomes[v]);
    }
    free(nomes);
    free(preciso);
    DijkstraResult_free(r);
    Graph_destroy(g);
    return EXIT_SUCCESS;
}
