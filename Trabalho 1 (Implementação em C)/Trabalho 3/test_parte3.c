/*
Testes de verificacao da Parte 3 (digrafos com pesos, Bellman-Ford e
Floyd-Warshall).

Rodar com:  ./test_parte3

Grafos de teste (valores conferidos a mao):

  graph_teste_direcionado.txt           -> digrafo com pesos positivos e
                                           alcance assimetrico (7 nao e
                                           alcancado por ninguem; 1 so e
                                           alcancado via 6->1)
  graph_teste_direcionado_negativo.txt  -> pesos negativos SEM ciclo
                                           negativo: d(1,4) = -2 e
                                           d(1,5) = -1 pela cadeia
                                           1->2->4->5
  graph_teste_direcionado_duplicatas.txt-> "1 2 5" e "1 2 3" (vale a
                                           ultima, 3), "2 1 9" (arco
                                           DIFERENTE, nao duplicata, no
                                           digrafo) e laco ignorado; lido
                                           tambem como NAO direcionado
                                           (ai "2 1 9" e que vale)
  graph_teste_ciclo_negativo.txt        -> ciclo negativo 2->3->2 alcancado
                                           do vertice 1 e do alvo 4
  graph_teste_ciclo_negativo_inalcancavel.txt -> ciclo negativo 3->4->3
                                           que o vertice 1 nao alcanca e
                                           que nao chega ao alvo 1
  graph_exemplo_direcionado.txt         -> a Figura 1 da Parte 2 lida como
                                           DIGRAFO: tem o ciclo negativo
                                           3->4->5->3 (-2.2)
  graph_exemplo.txt /                   -> regressao: os valores conhecidos
  graph_exemplo_pesos_positivos.txt        das Partes 1 e 2 continuam os
                                           mesmos quando o grafo e carregado
                                           como NAO direcionado
  graph_teste_rotulo_invalido.txt       -> aresta com vertice fora de
                                           [1, n]: a leitura recusa

O que os testes conferem, alem dos valores manuais:

  - a MESMA resposta nas duas representacoes (lista x matriz), para as
    buscas, o Dijkstra, o Bellman-Ford (fonte e alvo) e o Floyd-Warshall;
  - o Bellman-Ford concorda com o Dijkstra nos grafos sem pesos negativos
    (dist de cada fonte), e com a linha/coluna certa do Floyd-Warshall em
    todos os grafos em que as distancias sao confiaveis (sem ciclo
    negativo alcancado);
  - o Floyd-Warshall flaga ciclo negativo GLOBALMENTE; o Bellman-Ford so
    no que a fonte alcanca (ou no que chega ao alvo) - os testes
    exercitam os dois casos com o grafo do ciclo inalcancavel;
  - as operacoes da Parte 1 (componentes, distancia, diametro,
    estatisticas, arquivo de saida) recusam digrafos; em grafo nao
    direcionado, tudo continua como antes.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "graph.h"
#include "graph_utils.h"
#include "dijkstra.h"
#include "bellman_ford.h"
#include "floyd_warshall.h"
#include "johnson.h"

#define DIRECIONADO "graph_teste_direcionado.txt"
#define DIRECIONADO_NEGATIVO "graph_teste_direcionado_negativo.txt"
#define DIRECIONADO_DUPLICATAS "graph_teste_direcionado_duplicatas.txt"
#define CICLO_NEGATIVO "graph_teste_ciclo_negativo.txt"
#define CICLO_INALCANCAVEL "graph_teste_ciclo_negativo_inalcancavel.txt"
#define EXEMPLO_DIRECIONADO "graph_exemplo_direcionado.txt"
#define EXEMPLO "graph_exemplo.txt"
#define EXEMPLO_PESOS_POSITIVOS "graph_exemplo_pesos_positivos.txt"
#define ROTULO_INVALIDO "graph_teste_rotulo_invalido.txt"
/* 150 vertices (mais de um bloco de 64 no Floyd-Warshall em blocos), pesos
   negativos sem ciclo negativo: gerado por "Trabalho 3 - Estudos de caso/
   gerar_digrafos_teste.py" (truque do potencial, semente 42) */
#define DIGRAFO_150 "graph_teste_digrafo_150.txt"

#define MAX_FALHAS 64
#define MAX_MSG 256

static char falhas[MAX_FALHAS][MAX_MSG];
static int falhas_count = 0;

static void imprimir_resultado(const char *descricao, const char *obtido_str,
                                bool ok, const char *esperado_str) {
    printf("  [%s] %s: %s\n", ok ? "ok  " : "FALHA", descricao, obtido_str);
    if (!ok && falhas_count < MAX_FALHAS) {
        snprintf(falhas[falhas_count], MAX_MSG, "%s: obtido %s, esperado %s",
                 descricao, obtido_str, esperado_str);
        falhas_count += 1;
    }
}

static void checar_int(const char *descricao, int obtido, int esperado) {
    char obt[MAX_MSG], esp[MAX_MSG];
    snprintf(obt, MAX_MSG, "%d", obtido);
    snprintf(esp, MAX_MSG, "%d", esperado);
    imprimir_resultado(descricao, obt, obtido == esperado, esp);
}

static void checar_bool(const char *descricao, bool obtido, bool esperado) {
    imprimir_resultado(descricao, obtido ? "True" : "False", obtido == esperado,
                       esperado ? "True" : "False");
}

static void checar_string(const char *descricao, const char *obtido, const char *esperado) {
    imprimir_resultado(descricao, obtido, strcmp(obtido, esperado) == 0, esperado);
}

/* "v:peso v:peso" para comparar vizinhos (de SAIDA) e pesos */
static void vizinhos_pesos_para_string(Graph *g, int v, char *dest) {
    dest[0] = '\0';
    int primeiro = 1;
    int u;
    double peso;
    NeighborIter it = neighbor_iter(g, v);
    while (neighbor_next(&it, &u, &peso)) {
        char item[48];
        snprintf(item, sizeof item, "%s%d:%g", primeiro ? "" : " ", u, peso);
        strcat(dest, item);
        primeiro = 0;
    }
}

/* "[d0, d1, ...]" com %g (infinito aparece como "inf"): serve para o dist
   do Dijkstra, do Bellman-Ford e para cada linha do Floyd-Warshall */
static void dist_para_string(const double *dist, int n, char *dest) {
    dest[0] = '\0';
    strcat(dest, "[");
    for (int v = 0; v < n; v++) {
        char num[32];
        snprintf(num, sizeof num, (v == 0) ? "%g" : ", %g", dist[v]);
        strcat(dest, num);
    }
    strcat(dest, "]");
}

/* "[i0, i1, ...]" para comparar vetores de inteiros (pais, niveis, ordem) */
static void inteiros_para_string(const int *vetor, int n, char *dest) {
    dest[0] = '\0';
    strcat(dest, "[");
    for (int v = 0; v < n; v++) {
        char num[16];
        snprintf(num, sizeof num, (v == 0) ? "%d" : ", %d", vetor[v]);
        strcat(dest, num);
    }
    strcat(dest, "]");
}

/* checar_erro do test_parte1/test_parte2: roda a chamada em um processo
   filho e verifica se ela encerra com erro (ValueError -> exit(EXIT_FAILURE)) */
static void checar_erro(const char *descricao, void (*funcao)(void)) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        FILE *descartado = freopen("/dev/null", "w", stderr);
        (void)descartado;
        funcao();
        _exit(0); /* nao deveria chegar aqui */
    }
    int status = 0;
    waitpid(pid, &status, 0);
    bool morreu = WIFEXITED(status) && WEXITSTATUS(status) != 0;
    imprimir_resultado(descricao, morreu ? "encerrou com erro" : "sem excecao", morreu,
                       "erro (ValueError no Python)");
}

/* ------------------------------------------------------------------ */
/* leitura de digrafos                                                 */
/* ------------------------------------------------------------------ */

static void testar_leitura_direcionada(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(DIRECIONADO_DUPLICATAS, adj_list, true);

    snprintf(descricao, MAX_MSG, "flag directed [%s]", rep);
    checar_bool(descricao, g->directed, true);
    snprintf(descricao, MAX_MSG, "arco com peso != 1: weighted [%s]", rep);
    checar_bool(descricao, g->weighted, true);

    /* no digrafo, "1 2 5" + "1 2 3" = um arco 1->2 de peso 3 (vale a
       ultima linha); "2 1 9" e um arco DIFERENTE (2->1); o laco "3 3 4"
       e ignorado; 4->5 e 5->4 sao dois arcos */
    const char *esperado[5] = {"1:3", "0:9", "", "4:1", "3:1"};
    for (int v = 0; v < g->n; v++) {
        snprintf(descricao, MAX_MSG, "vizinhos de saida do vertice %d [%s]", v + 1, rep);
        vizinhos_pesos_para_string(g, v, obt);
        checar_string(descricao, obt, esperado[v]);
    }
    snprintf(descricao, MAX_MSG, "numero de arcos (duplicatas fundidas) [%s]", rep);
    checar_int(descricao, edge_count(g), 4);
    Graph_destroy(g);

    /* o MESMO arquivo como grafo nao direcionado: agora "1 2 3" e "2 1 9"
       sao a MESMA aresta 1-2, e vale a ultima linha (9); "1 2 5" e
       duplicata descartada antes */
    Graph *u = Graph_create(DIRECIONADO_DUPLICATAS, adj_list, false);
    snprintf(descricao, MAX_MSG, "mesmo arquivo nao direcionado: directed false [%s]", rep);
    checar_bool(descricao, u->directed, false);
    const char *esperado_u[5] = {"1:9", "0:9", "", "4:1", "3:1"};
    for (int v = 0; v < u->n; v++) {
        snprintf(descricao, MAX_MSG, "vizinhos do vertice %d, nao direcionado [%s]", v + 1, rep);
        vizinhos_pesos_para_string(u, v, obt);
        checar_string(descricao, obt, esperado_u[v]);
    }
    snprintf(descricao, MAX_MSG, "numero de arestas, nao direcionado [%s]", rep);
    checar_int(descricao, edge_count(u), 2);
    Graph_destroy(u);
}

/* ------------------------------------------------------------------ */
/* buscas em digrafos                                                  */
/* ------------------------------------------------------------------ */

static void testar_buscas_direcionadas(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(DIRECIONADO, adj_list, true);

    /* a direcao das arestas ja esta na leitura: as buscas nao tem uma
       linha de codigo propria para digrafo - seguem os arcos */

    /* BFS a partir de 1: 7 nunca e descoberto (ninguem vai ate ele) */
    GraphSearch *bfs = Graph_BFS(g, 1, 1, 0);
    snprintf(descricao, MAX_MSG, "BFS(1): niveis (7 inalcancavel) [%s]", rep);
    inteiros_para_string(bfs->levels, g->n, obt);
    checar_string(descricao, obt, "[0, 1, 1, 2, 2, 3, -1]");
    snprintf(descricao, MAX_MSG, "BFS(1): pais [%s]", rep);
    inteiros_para_string(bfs->parents, g->n, obt);
    checar_string(descricao, obt, "[-1, 0, 0, 1, 2, 4, -1]");
    snprintf(descricao, MAX_MSG, "BFS(1): ordem (6 vertices) [%s]", rep);
    inteiros_para_string(bfs->result_discovered, bfs->count, obt);
    checar_string(descricao, obt, "[0, 1, 2, 3, 4, 5]");
    GraphSearch_free(bfs);

    /* BFS a partir de 7: alcanca todo mundo */
    bfs = Graph_BFS(g, 7, 1, 0);
    snprintf(descricao, MAX_MSG, "BFS(7): niveis [%s]", rep);
    inteiros_para_string(bfs->levels, g->n, obt);
    checar_string(descricao, obt, "[1, 2, 2, 3, 3, 4, 0]");
    snprintf(descricao, MAX_MSG, "BFS(7): pais [%s]", rep);
    inteiros_para_string(bfs->parents, g->n, obt);
    checar_string(descricao, obt, "[6, 0, 0, 1, 2, 4, -1]");
    snprintf(descricao, MAX_MSG, "BFS(7): ordem (todos) [%s]", rep);
    inteiros_para_string(bfs->result_discovered, bfs->count, obt);
    checar_string(descricao, obt, "[6, 0, 1, 2, 3, 4, 5]");
    GraphSearch_free(bfs);

    /* DFS a partir de 1: desce 1->2->4->5->6 antes de voltar ao 3 */
    GraphSearch *dfs = Graph_DFS(g, 1, 1);
    snprintf(descricao, MAX_MSG, "DFS(1): ordem [%s]", rep);
    inteiros_para_string(dfs->result_discovered, dfs->count, obt);
    checar_string(descricao, obt, "[0, 1, 3, 4, 5, 2]");
    snprintf(descricao, MAX_MSG, "DFS(1): pais [%s]", rep);
    inteiros_para_string(dfs->parents, g->n, obt);
    checar_string(descricao, obt, "[-1, 0, 0, 1, 3, 4, -1]");
    snprintf(descricao, MAX_MSG, "DFS(1): niveis [%s]", rep);
    inteiros_para_string(dfs->levels, g->n, obt);
    checar_string(descricao, obt, "[0, 1, 1, 2, 3, 4, -1]");
    GraphSearch_free(dfs);

    /* DFS a partir de 7 */
    dfs = Graph_DFS(g, 7, 1);
    snprintf(descricao, MAX_MSG, "DFS(7): ordem [%s]", rep);
    inteiros_para_string(dfs->result_discovered, dfs->count, obt);
    checar_string(descricao, obt, "[6, 0, 1, 3, 4, 5, 2]");
    snprintf(descricao, MAX_MSG, "DFS(7): pais [%s]", rep);
    inteiros_para_string(dfs->parents, g->n, obt);
    checar_string(descricao, obt, "[6, 0, 0, 1, 3, 4, -1]");
    snprintf(descricao, MAX_MSG, "DFS(7): niveis [%s]", rep);
    inteiros_para_string(dfs->levels, g->n, obt);
    checar_string(descricao, obt, "[1, 2, 2, 3, 4, 5, 0]");
    GraphSearch_free(dfs);

    snprintf(descricao, MAX_MSG, "numero de arcos [%s]", rep);
    checar_int(descricao, edge_count(g), 9);
    Graph_destroy(g);
}

/* ------------------------------------------------------------------ */
/* dijkstra em digrafos                                                */
/* ------------------------------------------------------------------ */

static void testar_dijkstra_direcionado(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(DIRECIONADO, adj_list, true);

    for (int modo_i = 0; modo_i <= 1; modo_i++) {
        DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;

        /* do vertice 1: d(1,4) = 3+2 = 5 (por 2), d(1,5) = 5+1 = 6 (por
           4) e d(1,6) = 6+4 = 10 */
        DijkstraResult *r = dijkstra(g, 0, modo);
        snprintf(descricao, MAX_MSG, "dijkstra %s(1): distancias [%s]",
                 modo_i ? "heap" : "vetor", rep);
        dist_para_string(r->dist, g->n, obt);
        checar_string(descricao, obt, "[0, 3, 8, 5, 6, 10, inf]");
        snprintf(descricao, MAX_MSG, "dijkstra %s(1): pais [%s]",
                 modo_i ? "heap" : "vetor", rep);
        inteiros_para_string(r->parents, g->n, obt);
        checar_string(descricao, obt, "[-1, 0, 0, 1, 3, 4, -1]");
        DijkstraResult_free(r);

        /* do vertice 7: alcanca tudo; d(7,3) = 5+8 = 13, d(7,5) = 10+1 =
           11 (por 4) e d(7,6) = 11+4 = 15 */
        r = dijkstra(g, 6, modo);
        snprintf(descricao, MAX_MSG, "dijkstra %s(7): distancias [%s]",
                 modo_i ? "heap" : "vetor", rep);
        dist_para_string(r->dist, g->n, obt);
        checar_string(descricao, obt, "[5, 8, 13, 10, 11, 15, 0]");
        snprintf(descricao, MAX_MSG, "dijkstra %s(7): pais [%s]",
                 modo_i ? "heap" : "vetor", rep);
        inteiros_para_string(r->parents, g->n, obt);
        checar_string(descricao, obt, "[6, 0, 0, 1, 3, 4, -1]");

        /* caminho 7 -> 6: 7, 1, 2, 4, 5, 6 (custo 15) */
        int comprimento = 0;
        int *caminho = dijkstra_path(r, 5, &comprimento);
        snprintf(descricao, MAX_MSG, "dijkstra %s: caminho 7->6 [%s]",
                 modo_i ? "heap" : "vetor", rep);
        inteiros_para_string(caminho, comprimento, obt);
        checar_string(descricao, obt, "[6, 0, 1, 3, 4, 5]");
        free(caminho);
        DijkstraResult_free(r);

        /* do vertice 2: 7 e inalcancavel */
        r = dijkstra(g, 1, modo);
        snprintf(descricao, MAX_MSG, "dijkstra %s(2): distancias [%s]",
                 modo_i ? "heap" : "vetor", rep);
        dist_para_string(r->dist, g->n, obt);
        checar_string(descricao, obt, "[14, 0, 22, 2, 3, 7, inf]");
        caminho = dijkstra_path(r, 6, &comprimento);
        snprintf(descricao, MAX_MSG, "dijkstra %s: 7 inalcancavel a partir de 2 [%s]",
                 modo_i ? "heap" : "vetor", rep);
        checar_bool(descricao, caminho == NULL && comprimento == 0, true);
        free(caminho);
        DijkstraResult_free(r);
    }
    Graph_destroy(g);

    /* pesos negativos: o Dijkstra continua recusando (a Parte 3 resolve
       com Bellman-Ford/Floyd-Warshall) */
    Graph *neg = Graph_create(DIRECIONADO_NEGATIVO, adj_list, true);
    for (int modo_i = 0; modo_i <= 1; modo_i++) {
        DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
        snprintf(descricao, MAX_MSG, "dijkstra %s recusa peso negativo [%s]",
                 modo_i ? "heap" : "vetor", rep);
        checar_bool(descricao, dijkstra(neg, 0, modo) == NULL, true);
    }
    Graph_destroy(neg);
}

/* ------------------------------------------------------------------ */
/* bellman-ford                                                        */
/* ------------------------------------------------------------------ */

static void testar_bellman_ford(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(DIRECIONADO_NEGATIVO, adj_list, true);

    /* do vertice 1: d(1,4) = 1-3 = -2, d(1,5) = -2+1 = -1 */
    BellmanFordResult *r = bellman_ford(g, 0);
    snprintf(descricao, MAX_MSG, "bellman-ford(1): distancias com peso negativo [%s]", rep);
    dist_para_string(r->dist, g->n, obt);
    checar_string(descricao, obt, "[0, 1, 2, -2, -1]");
    snprintf(descricao, MAX_MSG, "bellman-ford(1): pais (arvore) [%s]", rep);
    inteiros_para_string(r->parents, g->n, obt);
    checar_string(descricao, obt, "[-1, 0, 0, 1, 3]");
    snprintf(descricao, MAX_MSG, "bellman-ford(1): sem ciclo negativo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);
    /* com a lista de ativos, a passada que so CONFIRMARIA que nada muda
       nao e feita: sem vertice ativo, ja se sabe. Antes eram 2 passadas
       (1 que relaxa + 1 de confirmacao); a ordem crescente resolve tudo
       na 1a. */
    snprintf(descricao, MAX_MSG, "bellman-ford(1): convergiu em 1 passada [%s]", rep);
    checar_int(descricao, r->passadas, 1);

    /* caminho 1 -> 5: 1, 2, 4, 5 (custo -1) */
    int comprimento = 0;
    int *caminho = bellman_ford_path(r, 4, &comprimento);
    snprintf(descricao, MAX_MSG, "bellman-ford: caminho 1->5 [%s]", rep);
    inteiros_para_string(caminho, comprimento, obt);
    checar_string(descricao, obt, "[0, 1, 3, 4]");
    free(caminho);
    BellmanFordResult_free(r);

    /* do vertice 2: so alcanca 4 e 5 */
    r = bellman_ford(g, 1);
    snprintf(descricao, MAX_MSG, "bellman-ford(2): distancias [%s]", rep);
    dist_para_string(r->dist, g->n, obt);
    checar_string(descricao, obt, "[inf, 0, inf, -3, -2]");
    snprintf(descricao, MAX_MSG, "bellman-ford(2): pais [%s]", rep);
    inteiros_para_string(r->parents, g->n, obt);
    checar_string(descricao, obt, "[-1, -1, -1, 1, 3]");
    BellmanFordResult_free(r);

    /* do vertice 5: sem arcos de saida; para na 1a passada */
    r = bellman_ford(g, 4);
    snprintf(descricao, MAX_MSG, "bellman-ford(5): tudo inalcancavel [%s]", rep);
    dist_para_string(r->dist, g->n, obt);
    checar_string(descricao, obt, "[inf, inf, inf, inf, 0]");
    snprintf(descricao, MAX_MSG, "bellman-ford(5): terminacao precoce na 1a passada [%s]", rep);
    checar_int(descricao, r->passadas, 1);
    BellmanFordResult_free(r);
    Graph_destroy(g);

    /* no digrafo POSITIVO: mesmas distancias do Dijkstra (d(1,5) = 6 por
       2->4, vencendo os 14 do caminho por 3) */
    Graph *p = Graph_create(DIRECIONADO, adj_list, true);
    r = bellman_ford(p, 0);
    snprintf(descricao, MAX_MSG, "bellman-ford(1) no digrafo positivo: distancias [%s]", rep);
    dist_para_string(r->dist, p->n, obt);
    checar_string(descricao, obt, "[0, 3, 8, 5, 6, 10, inf]");
    snprintf(descricao, MAX_MSG, "bellman-ford(1) no digrafo positivo: pais [%s]", rep);
    inteiros_para_string(r->parents, p->n, obt);
    checar_string(descricao, obt, "[-1, 0, 0, 1, 3, 4, -1]");
    snprintf(descricao, MAX_MSG, "bellman-ford(1) no digrafo positivo: 1 passada [%s]", rep);
    checar_int(descricao, r->passadas, 1);
    BellmanFordResult_free(r);

    r = bellman_ford(p, 6);
    snprintf(descricao, MAX_MSG, "bellman-ford(7) no digrafo positivo: distancias [%s]", rep);
    dist_para_string(r->dist, p->n, obt);
    checar_string(descricao, obt, "[5, 8, 13, 10, 11, 15, 0]");
    snprintf(descricao, MAX_MSG, "bellman-ford(7): 2 passadas (sem a de confirmacao) [%s]", rep);
    checar_int(descricao, r->passadas, 2);
    BellmanFordResult_free(r);
    Graph_destroy(p);
}

/* ------------------------------------------------------------------ */
/* bellman-ford "de todos para um"                                     */
/* ------------------------------------------------------------------ */

static void testar_bellman_ford_to(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(DIRECIONADO_NEGATIVO, adj_list, true);

    /* de TODOS para o 5: d(1,5) = -1, d(2,5) = -2, d(3,5) = 3, d(4,5) = 1.
       A arvore aponta para o alvo: parents[v] = o PROXIMO vertice no
       caminho de v ate o 5 (1->2, 2->4, 3->4, 4->5) */
    BellmanFordResult *r = bellman_ford_to(g, 4);
    snprintf(descricao, MAX_MSG, "bellman-ford-to(5): distancias [%s]", rep);
    dist_para_string(r->dist, g->n, obt);
    checar_string(descricao, obt, "[-1, -2, 3, 1, 0]");
    snprintf(descricao, MAX_MSG, "bellman-ford-to(5): arvore (proximos passos) [%s]", rep);
    inteiros_para_string(r->parents, g->n, obt);
    checar_string(descricao, obt, "[1, 3, 3, 4, -1]");
    snprintf(descricao, MAX_MSG, "bellman-ford-to(5): sem ciclo negativo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);

    /* caminho 1 -> 5 lendo a arvore para a frente: 1, 2, 4, 5 */
    int comprimento = 0;
    int *caminho = bellman_ford_path_to(r, 0, &comprimento);
    snprintf(descricao, MAX_MSG, "bellman-ford-to: caminho 1->5 [%s]", rep);
    inteiros_para_string(caminho, comprimento, obt);
    checar_string(descricao, obt, "[0, 1, 3, 4]");
    free(caminho);

    /* caminho 2 -> 5: 2, 4, 5 */
    caminho = bellman_ford_path_to(r, 1, &comprimento);
    snprintf(descricao, MAX_MSG, "bellman-ford-to: caminho 2->5 [%s]", rep);
    inteiros_para_string(caminho, comprimento, obt);
    checar_string(descricao, obt, "[1, 3, 4]");
    free(caminho);
    BellmanFordResult_free(r);

    /* de todos para o 3: so o 1 alcanca (direto, custo 2) */
    r = bellman_ford_to(g, 2);
    snprintf(descricao, MAX_MSG, "bellman-ford-to(3): distancias [%s]", rep);
    dist_para_string(r->dist, g->n, obt);
    checar_string(descricao, obt, "[2, inf, 0, inf, inf]");
    snprintf(descricao, MAX_MSG, "bellman-ford-to(3): arvore [%s]", rep);
    inteiros_para_string(r->parents, g->n, obt);
    checar_string(descricao, obt, "[2, -1, -1, -1, -1]");
    caminho = bellman_ford_path_to(r, 1, &comprimento); /* 2 nao alcanca 3 */
    snprintf(descricao, MAX_MSG, "bellman-ford-to: 2 nao alcanca 3 [%s]", rep);
    checar_bool(descricao, caminho == NULL && comprimento == 0, true);
    free(caminho);
    BellmanFordResult_free(r);
    Graph_destroy(g);

    /* no digrafo de 7 vertices, de todos para o 6: o unico arco que chega
       ao 6 e 5->6, entao d(v,6) = d(v,5) + 4 (d(1,5) = 6, d(7,5) = 11) */
    Graph *p = Graph_create(DIRECIONADO, adj_list, true);
    r = bellman_ford_to(p, 5);
    snprintf(descricao, MAX_MSG, "bellman-ford-to(6) no digrafo de 7: distancias [%s]", rep);
    dist_para_string(r->dist, p->n, obt);
    checar_string(descricao, obt, "[10, 7, 6, 5, 4, 0, 15]");
    snprintf(descricao, MAX_MSG, "bellman-ford-to(6) no digrafo de 7: arvore [%s]", rep);
    inteiros_para_string(r->parents, p->n, obt);
    checar_string(descricao, obt, "[1, 3, 3, 4, 5, -1, 0]");
    caminho = bellman_ford_path_to(r, 0, &comprimento);
    snprintf(descricao, MAX_MSG, "bellman-ford-to: caminho 1->6 [%s]", rep);
    inteiros_para_string(caminho, comprimento, obt);
    checar_string(descricao, obt, "[0, 1, 3, 4, 5]"); /* 1 -> 2 -> 4 -> 5 -> 6 */
    free(caminho);
    BellmanFordResult_free(r);
    Graph_destroy(p);
}

/* ------------------------------------------------------------------ */
/* ciclos negativos                                                    */
/* ------------------------------------------------------------------ */

static void testar_ciclos_negativos(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];

    /* ciclo 2->3->2 (custo -2), alcancado do vertice 1 */
    Graph *g = Graph_create(CICLO_NEGATIVO, adj_list, true);
    BellmanFordResult *r = bellman_ford(g, 0);
    snprintf(descricao, MAX_MSG, "BF(1): ciclo negativo alcancado da fonte [%s]", rep);
    checar_bool(descricao, r->negative_cycle, true);
    /* o ciclo 2->3->2 aparece no grafo dos pais ja na 1a passada (3 relaxa
       2 por -3): a deteccao antecipada (K&T 6.27) para ali, sem rodar as
       n-1 passadas */
    snprintf(descricao, MAX_MSG, "BF(1): ciclo detectado antes das n-1 passadas [%s]", rep);
    checar_bool(descricao, r->passadas < g->n - 1, true);
    BellmanFordResult_free(r);

    /* do vertice 4 (sem arcos de saida) o ciclo nao e alcancado */
    r = bellman_ford(g, 3);
    snprintf(descricao, MAX_MSG, "BF(4): ciclo fora do alcance da fonte [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);
    snprintf(descricao, MAX_MSG, "BF(4): distancias [%s]", rep);
    dist_para_string(r->dist, g->n, obt);
    checar_string(descricao, obt, "[inf, inf, inf, 0]");
    BellmanFordResult_free(r);

    /* de todos para o 4: o ciclo CHEGA ao 4 (3->4) -> flag */
    r = bellman_ford_to(g, 3);
    snprintf(descricao, MAX_MSG, "BF-to(4): ciclo que chega ao alvo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, true);
    BellmanFordResult_free(r);

    /* de todos para o 1: ninguem alcanca 1, e o ciclo nao chega ao 1 */
    r = bellman_ford_to(g, 0);
    snprintf(descricao, MAX_MSG, "BF-to(1): ciclo que nao chega ao alvo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);
    snprintf(descricao, MAX_MSG, "BF-to(1): distancias [%s]", rep);
    dist_para_string(r->dist, g->n, obt);
    checar_string(descricao, obt, "[0, inf, inf, inf]");
    BellmanFordResult_free(r);
    Graph_destroy(g);

    /* ciclo 3->4->3 (custo -2) que o vertice 1 NAO alcanca (e que nao
       chega ao vertice 1): o BF da fonte 1 e do alvo 1 nao flagam; o da
       fonte 3 e do alvo 5 (3->5) flagam; o Floyd-Warshall flaga sempre */
    Graph *h = Graph_create(CICLO_INALCANCAVEL, adj_list, true);
    r = bellman_ford(h, 0);
    snprintf(descricao, MAX_MSG, "BF(1): ciclo inalcancavel nao e flagado [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);
    snprintf(descricao, MAX_MSG, "BF(1): distancias (corretas) [%s]", rep);
    dist_para_string(r->dist, h->n, obt);
    checar_string(descricao, obt, "[0, 1, inf, inf, inf]");
    BellmanFordResult_free(r);

    r = bellman_ford(h, 2);
    snprintf(descricao, MAX_MSG, "BF(3): ciclo alcancado da fonte [%s]", rep);
    checar_bool(descricao, r->negative_cycle, true);
    BellmanFordResult_free(r);

    r = bellman_ford_to(h, 0);
    snprintf(descricao, MAX_MSG, "BF-to(1): ciclo nao chega ao alvo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);
    snprintf(descricao, MAX_MSG, "BF-to(1): distancias [%s]", rep);
    dist_para_string(r->dist, h->n, obt);
    checar_string(descricao, obt, "[0, 1, inf, inf, inf]");
    BellmanFordResult_free(r);

    r = bellman_ford_to(h, 4);
    snprintf(descricao, MAX_MSG, "BF-to(5): ciclo chega ao alvo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, true);
    BellmanFordResult_free(r);

    FloydWarshallResult *fw = floyd_warshall(h);
    snprintf(descricao, MAX_MSG, "floyd-warshall: flaga o ciclo GLOBALMENTE [%s]", rep);
    checar_bool(descricao, fw->negative_cycle, true);
    FloydWarshallResult_free(fw);
    Graph_destroy(h);

    /* a Figura 1 da Parte 2 como DIGRAFO: ciclo 3->4->5->3 = -2.2,
       alcancado do 1; ninguem volta ao 1 nem ao 2, entao BF-to nao flaga */
    Graph *f = Graph_create(EXEMPLO_DIRECIONADO, adj_list, true);
    r = bellman_ford(f, 0);
    snprintf(descricao, MAX_MSG, "BF(1) na Figura 1: ciclo negativo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, true);
    BellmanFordResult_free(r);

    r = bellman_ford_to(f, 0);
    snprintf(descricao, MAX_MSG, "BF-to(1) na Figura 1: ninguem chega ao 1 [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);
    snprintf(descricao, MAX_MSG, "BF-to(1) na Figura 1: distancias [%s]", rep);
    dist_para_string(r->dist, f->n, obt);
    checar_string(descricao, obt, "[0, inf, inf, inf, inf]");
    BellmanFordResult_free(r);

    r = bellman_ford_to(f, 1);
    snprintf(descricao, MAX_MSG, "BF-to(2) na Figura 1: so o 1 alcanca o 2 [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);
    snprintf(descricao, MAX_MSG, "BF-to(2) na Figura 1: distancias [%s]", rep);
    dist_para_string(r->dist, f->n, obt);
    checar_string(descricao, obt, "[0.1, 0, inf, inf, inf]");
    snprintf(descricao, MAX_MSG, "BF-to(2) na Figura 1: arvore [%s]", rep);
    inteiros_para_string(r->parents, f->n, obt);
    checar_string(descricao, obt, "[1, -1, -1, -1, -1]");
    BellmanFordResult_free(r);
    Graph_destroy(f);
}

/* ------------------------------------------------------------------ */
/* floyd-warshall                                                      */
/* ------------------------------------------------------------------ */

static void testar_floyd_warshall(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(DIRECIONADO_NEGATIVO, adj_list, true);

    FloydWarshallResult *r = floyd_warshall(g);
    snprintf(descricao, MAX_MSG, "floyd-warshall: sem ciclo negativo [%s]", rep);
    checar_bool(descricao, r->negative_cycle, false);

    /* distancias entre todos os pares (conferidas a mao) */
    const char *dist_esperado[5] = {
        "[0, 1, 2, -2, -1]",
        "[inf, 0, inf, -3, -2]",
        "[inf, inf, 0, 2, 3]",
        "[inf, inf, inf, 0, 1]",
        "[inf, inf, inf, inf, 0]",
    };
    const char *pred_esperado[5] = {
        "[-1, 0, 0, 1, 3]",
        "[-1, -1, -1, 1, 3]",
        "[-1, -1, -1, 2, 3]",
        "[-1, -1, -1, -1, 3]",
        "[-1, -1, -1, -1, -1]",
    };
    for (int i = 0; i < g->n; i++) {
        snprintf(descricao, MAX_MSG, "floyd-warshall: dist a partir de %d [%s]", i + 1, rep);
        dist_para_string(r->dist[i], g->n, obt);
        checar_string(descricao, obt, dist_esperado[i]);

        snprintf(descricao, MAX_MSG, "floyd-warshall: arvore (linha %d do pred) [%s]", i + 1, rep);
        inteiros_para_string(r->pred[i], g->n, obt);
        checar_string(descricao, obt, pred_esperado[i]);
    }

    /* caminhos reconstruidos: 1->5 = 1,2,4,5; 3->5 = 3,4,5; 2->3 nao existe */
    int comprimento = 0;
    int *caminho = floyd_warshall_path(r, 0, 4, &comprimento);
    snprintf(descricao, MAX_MSG, "floyd-warshall: caminho 1->5 [%s]", rep);
    inteiros_para_string(caminho, comprimento, obt);
    checar_string(descricao, obt, "[0, 1, 3, 4]");
    free(caminho);

    caminho = floyd_warshall_path(r, 2, 4, &comprimento);
    snprintf(descricao, MAX_MSG, "floyd-warshall: caminho 3->5 [%s]", rep);
    inteiros_para_string(caminho, comprimento, obt);
    checar_string(descricao, obt, "[2, 3, 4]");
    free(caminho);

    caminho = floyd_warshall_path(r, 1, 2, &comprimento);
    snprintf(descricao, MAX_MSG, "floyd-warshall: 2 nao alcanca 3 [%s]", rep);
    checar_bool(descricao, caminho == NULL && comprimento == 0, true);
    free(caminho);

    /* a coluna do alvo 5 = as distancias do bellman-ford-to(5) */
    BellmanFordResult *bt = bellman_ford_to(g, 4);
    bool coluna_igual = true;
    for (int i = 0; i < g->n; i++) {
        coluna_igual = coluna_igual && (bt->dist[i] == r->dist[i][4]);
    }
    snprintf(descricao, MAX_MSG, "floyd-warshall: coluna do alvo = BF-to(5) [%s]", rep);
    checar_bool(descricao, coluna_igual, true);
    BellmanFordResult_free(bt);
    FloydWarshallResult_free(r);
    Graph_destroy(g);

    /* no digrafo de 7 vertices (caminhos minimos unicos): a linha 1 do
       pred e a arvore do dijkstra/bellman-ford a partir de 1 */
    Graph *p = Graph_create(DIRECIONADO, adj_list, true);
    r = floyd_warshall(p);
    snprintf(descricao, MAX_MSG, "floyd-warshall(7 vertices): dist a partir de 1 [%s]", rep);
    dist_para_string(r->dist[0], p->n, obt);
    checar_string(descricao, obt, "[0, 3, 8, 5, 6, 10, inf]");
    snprintf(descricao, MAX_MSG, "floyd-warshall(7 vertices): arvore a partir de 1 [%s]", rep);
    inteiros_para_string(r->pred[0], p->n, obt);
    checar_string(descricao, obt, "[-1, 0, 0, 1, 3, 4, -1]");
    snprintf(descricao, MAX_MSG, "floyd-warshall(7 vertices): dist a partir de 7 [%s]", rep);
    dist_para_string(r->dist[6], p->n, obt);
    checar_string(descricao, obt, "[5, 8, 13, 10, 11, 15, 0]");
    snprintf(descricao, MAX_MSG, "floyd-warshall(7 vertices): arvore a partir de 7 [%s]", rep);
    inteiros_para_string(r->pred[6], p->n, obt);
    checar_string(descricao, obt, "[6, 0, 0, 1, 3, 4, -1]");
    FloydWarshallResult_free(r);
    Graph_destroy(p);
}

/* ------------------------------------------------------------------ */
/* grafo transposto                                                    */
/* ------------------------------------------------------------------ */

static void testar_graph_reverse(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(DIRECIONADO, adj_list, true);

    /* o transposto: quem chegava agora sai. Ex.: no grafo, 6->1 e 7->1;
       no transposto, 1 -> 6 e 1 -> 7 */
    Graph *t = Graph_reverse(g);
    const char *esperado[7] = {
        "5:7 6:5", /* 1: arcos de 6 (peso 7) e 7 (peso 5) */
        "0:3",     /* 2: arco de 1 (peso 3) */
        "0:8",     /* 3: arco de 1 (peso 8) */
        "1:2 2:1", /* 4: arcos de 2 (peso 2) e 3 (peso 1) */
        "2:6 3:1", /* 5: arcos de 3 (peso 6) e 4 (peso 1) */
        "4:4",     /* 6: arco de 5 (peso 4) */
        "",        /* 7: ninguem chega ao 7 */
    };
    for (int v = 0; v < t->n; v++) {
        snprintf(descricao, MAX_MSG, "transposto: vizinhos de saida do vertice %d [%s]", v + 1, rep);
        vizinhos_pesos_para_string(t, v, obt);
        checar_string(descricao, obt, esperado[v]);
    }

    /* o transposto do transposto e o grafo original */
    Graph *tt = Graph_reverse(t);
    bool igual = tt->n == g->n;
    for (int v = 0; v < g->n && igual; v++) {
        char a[MAX_MSG], b[MAX_MSG];
        vizinhos_pesos_para_string(g, v, a);
        vizinhos_pesos_para_string(tt, v, b);
        igual = strcmp(a, b) == 0;
    }
    snprintf(descricao, MAX_MSG, "transposto do transposto = original [%s]", rep);
    checar_bool(descricao, igual, true);
    Graph_destroy(tt);

    /* o bellman-ford-to e o bellman-ford no transposto: a distancia de
       v ao alvo 6 no grafo = a distancia de 6 a v no transposto */
    BellmanFordResult *r_to = bellman_ford_to(g, 5);
    BellmanFordResult *r_t = bellman_ford(t, 5);
    dist_para_string(r_to->dist, g->n, obt);
    char esp[MAX_MSG];
    dist_para_string(r_t->dist, t->n, esp);
    snprintf(descricao, MAX_MSG, "BF-to(6) no grafo = BF(6) no transposto [%s]", rep);
    checar_string(descricao, obt, esp);
    BellmanFordResult_free(r_to);
    BellmanFordResult_free(r_t);
    Graph_destroy(t);
    Graph_destroy(g);
}

/* ------------------------------------------------------------------ */
/* regressao: grafos nao direcionados continuam como nas Partes 1 e 2  */
/* ------------------------------------------------------------------ */

static void testar_nao_direcionado(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];

    /* graph_exemplo.txt sem pesos: os niveis da BFS da Parte 1 */
    Graph *g = Graph_create(EXEMPLO, adj_list, false);
    snprintf(descricao, MAX_MSG, "regressao: directed false [%s]", rep);
    checar_bool(descricao, g->directed, false);
    GraphSearch *bfs = Graph_BFS(g, 1, 1, 0);
    snprintf(descricao, MAX_MSG, "regressao: niveis da BFS no grafo sem pesos [%s]", rep);
    inteiros_para_string(bfs->levels, g->n, obt);
    checar_string(descricao, obt, "[0, 1, 2, 2, 1]");
    GraphSearch_free(bfs);
    Graph_destroy(g);

    /* graph_exemplo_pesos_positivos.txt: as distancias da Parte 2, agora
       pelo Dijkstra, pelo Bellman-Ford e pela linha 1 do Floyd-Warshall */
    Graph *p = Graph_create(EXEMPLO_PESOS_POSITIVOS, adj_list, false);
    DijkstraResult *dk = dijkstra(p, 0, DIJKSTRA_HEAP);
    snprintf(descricao, MAX_MSG, "regressao: dijkstra(1) [%s]", rep);
    dist_para_string(dk->dist, p->n, obt);
    checar_string(descricao, obt, "[0, 0.1, 5.3, 2.6, 0.3]");
    DijkstraResult_free(dk);

    BellmanFordResult *bf = bellman_ford(p, 0);
    snprintf(descricao, MAX_MSG, "regressao: bellman-ford(1) = dijkstra(1) [%s]", rep);
    dist_para_string(bf->dist, p->n, obt);
    checar_string(descricao, obt, "[0, 0.1, 5.3, 2.6, 0.3]");
    BellmanFordResult_free(bf);

    FloydWarshallResult *fw = floyd_warshall(p);
    snprintf(descricao, MAX_MSG, "regressao: floyd-warshall linha 1 [%s]", rep);
    dist_para_string(fw->dist[0], p->n, obt);
    checar_string(descricao, obt, "[0, 0.1, 5.3, 2.6, 0.3]");
    snprintf(descricao, MAX_MSG, "regressao: floyd-warshall sem ciclo negativo [%s]", rep);
    checar_bool(descricao, fw->negative_cycle, false);
    FloydWarshallResult_free(fw);

    /* o bellman-ford-to tambem vale para grafo nao direcionado: o
       transposto e o proprio grafo (d(v,4) = d(4,v)) */
    BellmanFordResult *bt = bellman_ford_to(p, 3);
    snprintf(descricao, MAX_MSG, "regressao: BF-to(4) em grafo nao direcionado [%s]", rep);
    dist_para_string(bt->dist, p->n, obt);
    checar_string(descricao, obt, "[2.6, 2.5, 7.3, 0, 2.3]");
    snprintf(descricao, MAX_MSG, "regressao: BF-to(4): arvore [%s]", rep);
    inteiros_para_string(bt->parents, p->n, obt);
    checar_string(descricao, obt, "[1, 4, 4, -1, 3]");
    int comprimento = 0;
    int *caminho = bellman_ford_path_to(bt, 2, &comprimento);
    snprintf(descricao, MAX_MSG, "regressao: BF-to(4): caminho 3->4 [%s]", rep);
    inteiros_para_string(caminho, comprimento, obt);
    checar_string(descricao, obt, "[2, 4, 3]"); /* 3 -> 5 -> 4 */
    free(caminho);
    BellmanFordResult_free(bt);
    Graph_destroy(p);
}

/* ------------------------------------------------------------------ */
/* equivalencia entre representacoes e entre algoritmos                */
/* ------------------------------------------------------------------ */

static const char *ARQUIVOS_DIRECIONADOS[] = {
    DIRECIONADO,
    DIRECIONADO_NEGATIVO,
    DIRECIONADO_DUPLICATAS,
    EXEMPLO_DIRECIONADO,
    CICLO_NEGATIVO,
    CICLO_INALCANCAVEL,
};

static void testar_equivalencia_representacoes(void) {
    char descricao[MAX_MSG];
    char obt[MAX_MSG], esp[MAX_MSG];

    int arquivos = (int)(sizeof ARQUIVOS_DIRECIONADOS / sizeof *ARQUIVOS_DIRECIONADOS);
    for (int f = 0; f < arquivos; f++) {
        Graph *gl = Graph_create(ARQUIVOS_DIRECIONADOS[f], true, true);
        Graph *gm = Graph_create(ARQUIVOS_DIRECIONADOS[f], false, true);

        for (int inicio = 0; inicio < gl->n; inicio++) {
            GraphSearch *sl = Graph_BFS(gl, inicio + 1, 1, 0);
            GraphSearch *sm = Graph_BFS(gm, inicio + 1, 1, 0);
            snprintf(descricao, MAX_MSG, "BFS niveis iguais (%s, inicio %d)",
                     ARQUIVOS_DIRECIONADOS[f], inicio + 1);
            inteiros_para_string(sl->levels, gl->n, obt);
            inteiros_para_string(sm->levels, gm->n, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
            snprintf(descricao, MAX_MSG, "BFS pais iguais (%s, inicio %d)",
                     ARQUIVOS_DIRECIONADOS[f], inicio + 1);
            inteiros_para_string(sl->parents, gl->n, obt);
            inteiros_para_string(sm->parents, gm->n, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
            GraphSearch_free(sl);
            GraphSearch_free(sm);

            GraphSearch *dl = Graph_DFS(gl, inicio + 1, 1);
            GraphSearch *dm = Graph_DFS(gm, inicio + 1, 1);
            snprintf(descricao, MAX_MSG, "DFS ordem igual (%s, inicio %d)",
                     ARQUIVOS_DIRECIONADOS[f], inicio + 1);
            inteiros_para_string(dl->result_discovered, dl->count, obt);
            inteiros_para_string(dm->result_discovered, dm->count, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
            GraphSearch_free(dl);
            GraphSearch_free(dm);

            BellmanFordResult *bl = bellman_ford(gl, inicio);
            BellmanFordResult *bm = bellman_ford(gm, inicio);
            snprintf(descricao, MAX_MSG, "BF dist iguais (%s, fonte %d)",
                     ARQUIVOS_DIRECIONADOS[f], inicio + 1);
            dist_para_string(bl->dist, gl->n, obt);
            dist_para_string(bm->dist, gm->n, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
            snprintf(descricao, MAX_MSG, "BF pais iguais (%s, fonte %d)",
                     ARQUIVOS_DIRECIONADOS[f], inicio + 1);
            inteiros_para_string(bl->parents, gl->n, obt);
            inteiros_para_string(bm->parents, gm->n, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
            snprintf(descricao, MAX_MSG, "BF mesmo veredito de ciclo (%s, fonte %d)",
                     ARQUIVOS_DIRECIONADOS[f], inicio + 1);
            imprimir_resultado(descricao, bl->negative_cycle ? "True" : "False",
                               bl->negative_cycle == bm->negative_cycle,
                               bm->negative_cycle ? "True" : "False");
            snprintf(descricao, MAX_MSG, "BF passadas iguais (%s, fonte %d)",
                     ARQUIVOS_DIRECIONADOS[f], inicio + 1);
            snprintf(obt, MAX_MSG, "%d", bl->passadas);
            snprintf(esp, MAX_MSG, "%d", bm->passadas);
            imprimir_resultado(descricao, obt, bl->passadas == bm->passadas, esp);
            BellmanFordResult_free(bl);
            BellmanFordResult_free(bm);
        }

        for (int alvo = 0; alvo < gl->n; alvo++) {
            BellmanFordResult *bl = bellman_ford_to(gl, alvo);
            BellmanFordResult *bm = bellman_ford_to(gm, alvo);
            snprintf(descricao, MAX_MSG, "BF-to dist iguais (%s, alvo %d)",
                     ARQUIVOS_DIRECIONADOS[f], alvo + 1);
            dist_para_string(bl->dist, gl->n, obt);
            dist_para_string(bm->dist, gm->n, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
            BellmanFordResult_free(bl);
            BellmanFordResult_free(bm);
        }

        /* o dijkstra so roda sem pesos negativos */
        if (!gl->negative_weight) {
            for (int inicio = 0; inicio < gl->n; inicio++) {
                for (int modo_i = 0; modo_i <= 1; modo_i++) {
                    DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
                    DijkstraResult *dl = dijkstra(gl, inicio, modo);
                    DijkstraResult *dm = dijkstra(gm, inicio, modo);
                    snprintf(descricao, MAX_MSG, "dijkstra %s dist iguais (%s, inicio %d)",
                             modo_i ? "heap" : "vetor", ARQUIVOS_DIRECIONADOS[f], inicio + 1);
                    dist_para_string(dl->dist, gl->n, obt);
                    dist_para_string(dm->dist, gm->n, esp);
                    imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
                    DijkstraResult_free(dl);
                    DijkstraResult_free(dm);
                }
            }
        }

        FloydWarshallResult *fl = floyd_warshall(gl);
        FloydWarshallResult *fm = floyd_warshall(gm);
        bool fw_igual = fl->negative_cycle == fm->negative_cycle;
        for (int i = 0; i < gl->n && fw_igual; i++) {
            for (int j = 0; j < gl->n; j++) {
                char a[32], b[32];
                snprintf(a, sizeof a, "%g", fl->dist[i][j]);
                snprintf(b, sizeof b, "%g", fm->dist[i][j]);
                fw_igual = strcmp(a, b) == 0;
                if (!fw_igual) {
                    break;
                }
            }
        }
        snprintf(descricao, MAX_MSG, "floyd-warshall inteiro igual (%s)",
                 ARQUIVOS_DIRECIONADOS[f]);
        imprimir_resultado(descricao, fw_igual ? "igual" : "diferente", fw_igual, "igual");
        FloydWarshallResult_free(fl);
        FloydWarshallResult_free(fm);

        Graph_destroy(gl);
        Graph_destroy(gm);
    }
}

static void testar_equivalencia_entre_algoritmos(void) {
    /* dijkstra = bellman-ford = linha do floyd-warshall (sem ciclo
       negativo no caminho); BF-to = coluna do floyd-warshall */
    char descricao[MAX_MSG];
    char obt[MAX_MSG], esp[MAX_MSG];

    int arquivos = (int)(sizeof ARQUIVOS_DIRECIONADOS / sizeof *ARQUIVOS_DIRECIONADOS);
    for (int f = 0; f < arquivos; f++) {
        Graph *g = Graph_create(ARQUIVOS_DIRECIONADOS[f], true, true);
        FloydWarshallResult *fw = floyd_warshall(g);

        for (int inicio = 0; inicio < g->n; inicio++) {
            BellmanFordResult *bf = bellman_ford(g, inicio);

            /* sem ciclo negativo alcancado, o BF e confiavel e tem que
               bater com a linha do Floyd-Warshall */
            if (!bf->negative_cycle) {
                snprintf(descricao, MAX_MSG, "BF(%d) = linha %d do FW (%s)",
                         inicio + 1, inicio + 1, ARQUIVOS_DIRECIONADOS[f]);
                dist_para_string(bf->dist, g->n, obt);
                dist_para_string(fw->dist[inicio], g->n, esp);
                imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
            }

            /* sem pesos negativos, o dijkstra tambem tem que bater */
            if (!g->negative_weight) {
                DijkstraResult *dk = dijkstra(g, inicio, DIJKSTRA_HEAP);
                snprintf(descricao, MAX_MSG, "BF(%d) = dijkstra(%d) (%s)",
                         inicio + 1, inicio + 1, ARQUIVOS_DIRECIONADOS[f]);
                dist_para_string(bf->dist, g->n, obt);
                dist_para_string(dk->dist, g->n, esp);
                imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
                DijkstraResult_free(dk);
            }
            BellmanFordResult_free(bf);
        }

        for (int alvo = 0; alvo < g->n; alvo++) {
            BellmanFordResult *bt = bellman_ford_to(g, alvo);
            if (!bt->negative_cycle) {
                bool coluna_igual = true;
                for (int i = 0; i < g->n; i++) {
                    coluna_igual = coluna_igual && (bt->dist[i] == fw->dist[i][alvo]);
                }
                snprintf(descricao, MAX_MSG, "BF-to(%d) = coluna %d do FW (%s)",
                         alvo + 1, alvo + 1, ARQUIVOS_DIRECIONADOS[f]);
                imprimir_resultado(descricao, coluna_igual ? "igual" : "diferente",
                                   coluna_igual, "igual");
            }
            BellmanFordResult_free(bt);
        }

        /* Johnson (todos os pares por potencial + dijkstra): detecta o
           mesmo ciclo negativo que o FW (os dois olham o grafo inteiro) e,
           sem ciclo, cada fonte e a linha do FW */
        Johnson *jo = johnson_preparar(g);
        snprintf(descricao, MAX_MSG, "johnson: ciclo negativo = FW (%s)", ARQUIVOS_DIRECIONADOS[f]);
        imprimir_resultado(descricao, jo->negative_cycle ? "sim" : "nao",
                           jo->negative_cycle == fw->negative_cycle,
                           fw->negative_cycle ? "sim" : "nao");
        if (!jo->negative_cycle) {
            for (int inicio = 0; inicio < g->n; inicio++) {
                DijkstraResult *dj = johnson_fonte(jo, g, inicio);
                snprintf(descricao, MAX_MSG, "johnson(%d) = linha %d do FW (%s)",
                         inicio + 1, inicio + 1, ARQUIVOS_DIRECIONADOS[f]);
                dist_para_string(dj->dist, g->n, obt);
                dist_para_string(fw->dist[inicio], g->n, esp);
                imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);
                DijkstraResult_free(dj);
            }
        }
        Johnson_free(jo);

        FloydWarshallResult_free(fw);
        Graph_destroy(g);
    }
}

/* ------------------------------------------------------------------ */
/* digrafo com 150 vertices: o Floyd-Warshall EM BLOCOS (3 blocos de 64) */
/* ------------------------------------------------------------------ */

/* custo do caminho i -> j lido da matriz de predecessores, pelas arestas do
   grafo; NAN se a arvore nao leva de volta a i em ate n passos */
static double custo_pelo_pred(Graph *g, FloydWarshallResult *fw, int i, int j) {
    double soma = 0.0;
    int v = j;
    for (int passos = 0; v != i; passos++) {
        int u = fw->pred[i][v];
        if (u < 0 || passos > g->n) {
            return NAN;
        }
        double peso_uv = NAN;
        int w;
        double peso;
        NeighborIter it = neighbor_iter(g, u);
        while (neighbor_next(&it, &w, &peso)) {
            if (w == v && (isnan(peso_uv) || peso < peso_uv)) {
                peso_uv = peso;
            }
        }
        if (isnan(peso_uv)) {
            return NAN; /* pred aponta para aresta que nao existe */
        }
        soma += peso_uv;
        v = u;
    }
    return soma;
}

static void testar_digrafo_150(void) {
    /* com n > 64, o FW roda em blocos (ordem diferente da classica): as
       distancias tem que bater com o BF de cada fonte e com o Johnson, e
       todo caminho reconstruido pelos predecessores tem que custar a
       distancia (em empate, o FW em blocos pode escolher OUTRO caminho
       minimo - por isso se confere o custo, nao o predecessor) */
    char descricao[MAX_MSG];
    Graph *g = Graph_create(DIGRAFO_150, true, true);
    FloydWarshallResult *fw = floyd_warshall(g);
    Johnson *jo = johnson_preparar(g);

    checar_bool("digrafo 150: FW sem ciclo negativo", fw->negative_cycle, false);
    checar_bool("digrafo 150: johnson sem ciclo negativo", jo->negative_cycle, false);

    double dif_bf = 0.0, dif_jo = 0.0, dif_caminho = 0.0;
    int alcance_diferente = 0, caminhos_invalidos = 0;
    for (int s = 0; s < g->n; s++) {
        BellmanFordResult *bf = bellman_ford(g, s);
        DijkstraResult *dj = johnson_fonte(jo, g, s);
        for (int v = 0; v < g->n; v++) {
            double f = fw->dist[s][v];
            bool inf_f = f == INFINITO_PESO;
            if (inf_f != (bf->dist[v] == INFINITO_PESO) || inf_f != (dj->dist[v] == INFINITO_PESO)) {
                alcance_diferente += 1;
                continue;
            }
            if (inf_f || s == v) {
                continue;
            }
            dif_bf = fmax(dif_bf, fabs(f - bf->dist[v]));
            dif_jo = fmax(dif_jo, fabs(f - dj->dist[v]));
            double c = custo_pelo_pred(g, fw, s, v);
            if (isnan(c)) {
                caminhos_invalidos += 1;
            } else {
                dif_caminho = fmax(dif_caminho, fabs(c - f));
            }
        }
        BellmanFordResult_free(bf);
        DijkstraResult_free(dj);
    }

    checar_int("digrafo 150: mesma alcancabilidade em FW, BF e johnson", alcance_diferente, 0);
    snprintf(descricao, MAX_MSG, "digrafo 150: FW = BF de cada fonte (maior diferenca %.1e)", dif_bf);
    checar_bool(descricao, dif_bf < 1e-9, true);
    snprintf(descricao, MAX_MSG, "digrafo 150: FW = johnson (maior diferenca %.1e)", dif_jo);
    checar_bool(descricao, dif_jo < 1e-9, true);
    checar_int("digrafo 150: todo caminho do pred do FW existe no grafo", caminhos_invalidos, 0);
    snprintf(descricao, MAX_MSG, "digrafo 150: custo de cada caminho do pred = distancia (%.1e)", dif_caminho);
    checar_bool(descricao, dif_caminho < 1e-9, true);

    /* o modo economico (distancias em float): mesmas distancias ate a
       precisao do float, mesma alcancabilidade, mesmo veredito de ciclo */
    FloydWarshallResult *fe = floyd_warshall_economico(g);
    double dif_f = 0.0;
    int alcance_f = 0;
    for (int s = 0; s < g->n; s++) {
        for (int v = 0; v < g->n; v++) {
            double a = fw->dist[s][v], b = floyd_warshall_dist(fe, s, v);
            if ((a == INFINITO_PESO) != (b == INFINITO_PESO)) {
                alcance_f += 1;
            } else if (a != INFINITO_PESO) {
                dif_f = fmax(dif_f, fabs(a - b));
            }
        }
    }
    checar_bool("digrafo 150: FW economico sem ciclo negativo", fe->negative_cycle, false);
    checar_int("digrafo 150: FW economico com a mesma alcancabilidade", alcance_f, 0);
    snprintf(descricao, MAX_MSG, "digrafo 150: FW economico = FW (maior diferenca %.1e, float)", dif_f);
    checar_bool(descricao, dif_f < 1e-3, true);
    FloydWarshallResult_free(fe);

    Graph *c = Graph_create(CICLO_NEGATIVO, true, true);
    fe = floyd_warshall_economico(c);
    checar_bool("FW economico: detecta o ciclo negativo", fe->negative_cycle, true);
    FloydWarshallResult_free(fe);
    Graph_destroy(c);

    Johnson_free(jo);
    FloydWarshallResult_free(fw);
    Graph_destroy(g);
}

/* ------------------------------------------------------------------ */
/* rotulos invalidos, digrafos recusados e arquivo invalido            */
/* ------------------------------------------------------------------ */

static Graph *g_erro = NULL;

static void erro_bellman_ford_99(void) { Graph_bellman_ford(g_erro, 99); }
static void erro_bellman_ford_0(void) { Graph_bellman_ford(g_erro, 0); }
static void erro_bellman_ford_to_99(void) { Graph_bellman_ford_to(g_erro, 99); }
static void erro_componentes(void) { Graph_connected_components(g_erro); }
static void erro_distancia(void) { Graph_distance(g_erro, 1, 2); }
static void erro_diametro(void) { Graph_diameter(g_erro, 0); }
static void erro_stats(void) { Graph_stats(g_erro); }
static void erro_write_output(void) { Graph_write_output(g_erro, "nao_deve_existir.txt"); }
static void erro_leitura_lista(void) { Graph_create(ROTULO_INVALIDO, true, true); }
static void erro_leitura_matriz(void) { Graph_create(ROTULO_INVALIDO, false, false); }

static void testar_rotulos_invalidos(void) {
    g_erro = Graph_create(DIRECIONADO, true, true);
    checar_erro("Graph_bellman_ford rejeita rotulo 99", erro_bellman_ford_99);
    checar_erro("Graph_bellman_ford rejeita rotulo 0", erro_bellman_ford_0);
    checar_erro("Graph_bellman_ford_to rejeita rotulo 99", erro_bellman_ford_to_99);
    Graph_destroy(g_erro);

    /* a Parte 1 e para grafos nao direcionados: num digrafo, recusa */
    g_erro = Graph_create(DIRECIONADO, true, true);
    checar_erro("componentes conexas recusam digrafo", erro_componentes);
    checar_erro("distancia recusa digrafo", erro_distancia);
    checar_erro("diametro recusa digrafo", erro_diametro);
    checar_erro("estatisticas recusam digrafo", erro_stats);
    checar_erro("arquivo de saida recusa digrafo", erro_write_output);
    Graph_destroy(g_erro);
    g_erro = NULL;

    /* aresta com vertice fora de [1, n]: a leitura para com erro */
    checar_erro("leitura recusa vertice fora de [1, n] (lista)", erro_leitura_lista);
    checar_erro("leitura recusa vertice fora de [1, n] (matriz)", erro_leitura_matriz);
}

/* ------------------------------------------------------------------ */
/* wrappers Graph_* (rotulos 1-based)                                  */
/* ------------------------------------------------------------------ */

static void testar_wrapper_graph(void) {
    char obt[MAX_MSG];

    Graph *g = Graph_create(DIRECIONADO_NEGATIVO, true, true);
    BellmanFordResult *r = Graph_bellman_ford(g, 1);
    dist_para_string(r->dist, g->n, obt);
    checar_string("Graph_bellman_ford(1): distancias", obt, "[0, 1, 2, -2, -1]");

    BellmanFordResult *rt = Graph_bellman_ford_to(g, 5);
    dist_para_string(rt->dist, g->n, obt);
    checar_string("Graph_bellman_ford_to(5): distancias", obt, "[-1, -2, 3, 1, 0]");

    FloydWarshallResult *fw = Graph_floyd_warshall(g);
    dist_para_string(fw->dist[0], g->n, obt);
    checar_string("Graph_floyd_warshall: linha 1", obt, "[0, 1, 2, -2, -1]");

    /* o transposto pelo wrapper tem o mesmo n e os arcos invertidos */
    Graph *t = Graph_reverse(g);
    char esp[MAX_MSG];
    vizinhos_pesos_para_string(t, 0, esp); /* no transposto, 1 so recebe... nada? */
    /* no grafo, quem chega ao 1? ninguem: 1 nao recebe arcos */
    checar_string("Graph_reverse: vertice 1 sem arcos de saida no transposto", esp, "");
    BellmanFordResult_free(r);
    BellmanFordResult_free(rt);
    FloydWarshallResult_free(fw);
    Graph_destroy(t);
    Graph_destroy(g);
}

int main(void) {
    const char *representacoes[2] = {"matriz de adjacencia", "lista de adjacencia"};

    for (int i = 0; i < 2; i++) {
        bool adj_list = (i == 1);

        printf("\n=== graph_teste_direcionado_duplicatas.txt / %s ===\n", representacoes[i]);
        testar_leitura_direcionada(adj_list);

        printf("\n=== graph_teste_direcionado.txt: buscas / %s ===\n", representacoes[i]);
        testar_buscas_direcionadas(adj_list);

        printf("\n=== graph_teste_direcionado.txt: dijkstra / %s ===\n", representacoes[i]);
        testar_dijkstra_direcionado(adj_list);

        printf("\n=== bellman-ford / %s ===\n", representacoes[i]);
        testar_bellman_ford(adj_list);

        printf("\n=== bellman-ford de todos para um / %s ===\n", representacoes[i]);
        testar_bellman_ford_to(adj_list);

        printf("\n=== ciclos negativos / %s ===\n", representacoes[i]);
        testar_ciclos_negativos(adj_list);

        printf("\n=== floyd-warshall / %s ===\n", representacoes[i]);
        testar_floyd_warshall(adj_list);

        printf("\n=== grafo transposto / %s ===\n", representacoes[i]);
        testar_graph_reverse(adj_list);

        printf("\n=== regressao: grafos nao direcionados / %s ===\n", representacoes[i]);
        testar_nao_direcionado(adj_list);
    }

    printf("\n=== equivalencia entre representacoes ===\n");
    testar_equivalencia_representacoes();

    printf("\n=== equivalencia entre algoritmos (BF x dijkstra x FW x johnson) ===\n");
    testar_equivalencia_entre_algoritmos();

    printf("\n=== digrafo com 150 vertices: FW em blocos x BF x johnson ===\n");
    testar_digrafo_150();

    printf("\n=== rotulos invalidos e digrafos recusados ===\n");
    testar_rotulos_invalidos();

    printf("\n=== wrappers Graph_* ===\n");
    testar_wrapper_graph();

    printf("\n==================================================\n");
    if (falhas_count > 0) {
        printf("%d FALHA(S):\n", falhas_count);
        for (int i = 0; i < falhas_count; i++) {
            printf("  - %s\n", falhas[i]);
        }
        return 1;
    }
    printf("Todos os testes passaram.\n");
    return 0;
}

