/*
Questao 1 do estudo de caso (secao 4 do enunciado): comparacao da memoria
usada pelas duas representacoes.

Carrega o grafo na representacao escolhida e mede a memoria residente do
processo (VmRSS em /proc/self/status, a mesma coluna RES do top) antes e
depois da carga: a diferenca e a memoria ocupada pelo grafo.

  pausa (padrao)  depois de medir, pausa a execucao para a conferencia
                  MANUAL (top/pmap em outra janela); ENTER encerra.
  auto            so imprime as medidas e encerra (usado por
                  rodar_estudos.sh).

Na matriz, o calloc entrega paginas zeradas sob demanda, e paginas que
nunca recebem uma aresta nao chegam a ocupar memoria fisica. Por isso o
programa imprime tambem o tamanho alocado para a matriz.

MB aqui e 1024 * 1024 bytes. Fora do Linux (sem /proc) as medidas
automaticas nao estao disponiveis; use a pausa e o gerenciador de tarefas.

uso: ./bin/questao1 <grafo.txt> <matriz|lista> [pausa|auto]
exemplo com um grafo do estudo de caso (pasta Grafos na raiz do repositorio):
    ./bin/questao1 ../Grafos/grafo_1.txt lista
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graph.h"

static void uso(const char *programa) {
    fprintf(stderr, "uso: %s <grafo.txt> <matriz|lista> [pausa|auto]\n", programa);
    exit(EXIT_FAILURE);
}

/* valor, em kB, de um campo de /proc/self/status ("VmRSS", "VmHWM");
   -1 quando o arquivo nao existe (fora do Linux) */
static long memoria_kb(const char *campo) {
    FILE *status = fopen("/proc/self/status", "r");
    if (status == NULL) {
        return -1;
    }

    char linha[256];
    size_t tamanho = strlen(campo);
    long valor = -1;
    while (fgets(linha, sizeof linha, status) != NULL) {
        if (strncmp(linha, campo, tamanho) == 0 && linha[tamanho] == ':') {
            valor = strtol(linha + tamanho + 1, NULL, 10);
            break;
        }
    }
    fclose(status);
    return valor;
}

static void imprimir_mb(const char *descricao, double kb) {
    if (kb < 0) {
        printf("%s: indisponivel (sem /proc/self/status)\n", descricao);
    } else {
        printf("%s: %.1f MB\n", descricao, kb / 1024.0);
    }
}

int main(int argc, char **argv) {
    if (argc != 3 && argc != 4) {
        uso(argv[0]);
    }

    bool adj_list;
    if (strcmp(argv[2], "lista") == 0) {
        adj_list = true;
    } else if (strcmp(argv[2], "matriz") == 0) {
        adj_list = false;
    } else {
        fprintf(stderr, "representacao invalida: %s (use matriz ou lista)\n", argv[2]);
        return EXIT_FAILURE;
    }

    bool pausar = true;
    if (argc == 4) {
        if (strcmp(argv[3], "auto") == 0) {
            pausar = false;
        } else if (strcmp(argv[3], "pausa") != 0) {
            fprintf(stderr, "modo invalido: %s (use pausa ou auto)\n", argv[3]);
            return EXIT_FAILURE;
        }
    }

    long antes = memoria_kb("VmRSS");
    Graph *g = Graph_create(argv[1], adj_list);
    long depois = memoria_kb("VmRSS");

    printf("Grafo carregado: %d vertices | representacao: %s de adjacencia\n",
           g->n, adj_list ? "lista" : "matriz");
    imprimir_mb("memoria residente antes de carregar", (double)antes);
    imprimir_mb("memoria residente depois de carregar", (double)depois);
    imprimir_mb("memoria do grafo (diferenca)",
                (antes < 0 || depois < 0) ? -1.0 : (double)(depois - antes));
    imprimir_mb("pico de memoria residente (VmHWM)", (double)memoria_kb("VmHWM"));
    if (!adj_list) {
        double bytes = (double)g->n * g->n * sizeof(MatrixCell)
                       + (double)g->n * sizeof(MatrixCell *);
        imprimir_mb("alocado para a matriz (n*n celulas de 1 byte)", bytes / 1024.0);
    }

    if (pausar) {
        printf("Measure memory.\n");
        printf("(pressione ENTER para encerrar)\n");
        fflush(stdout);
        getchar();
    }

    Graph_destroy(g);
    return EXIT_SUCCESS;
}
