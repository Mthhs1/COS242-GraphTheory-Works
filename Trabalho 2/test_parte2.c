/*
Testes de verificacao da Parte 2 (grafos com pesos e Dijkstra).

Rodar com:  ./test_parte2

Grafos de teste (valores conferidos a mao):

  graph_exemplo_pesos.txt           -> a Figura 1 da Parte 2 do enunciado,
                                       com a aresta 3-4 de peso -9.5: serve
                                       para a recusa de pesos negativos
  graph_exemplo_pesos_positivos.txt -> o mesmo grafo com peso +9.5:
                                       d(1,2)=0.1, d(1,5)=0.3, d(1,4)=2.6,
                                       d(1,3)=5.3
  graph_teste_pesos.txt             -> duplicata de aresta com pesos
                                       diferentes ("1 2" sem peso e "1 2
                                       2.5": vale a ultima linha, 2.5, nas
                                       duas representacoes) + laco com peso
  graph_teste_pesos_desconexo.txt   -> tres componentes com pesos; testa
                                       inalcancaveis (dist infinita)

O Dijkstra com vetor e o com heap tem que dar exatamente a mesma resposta
(as duas formas guardam as estimativas; o algoritmo e um so), e a resposta
nao pode depender da representacao do grafo - os testes conferem os dois.
Em grafo SEM pesos (arquivo de duas colunas), o Dijkstra tem que reproduzir
os niveis da BFS: toda aresta vale 1.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "graph.h"
#include "graph_utils.h"
#include "heap.h"
#include "dijkstra.h"

#define EXEMPLO "graph_exemplo.txt"
#define EXEMPLO_PESOS "graph_exemplo_pesos.txt"
#define EXEMPLO_PESOS_POSITIVOS "graph_exemplo_pesos_positivos.txt"
#define TESTE_PESOS "graph_teste_pesos.txt"
#define TESTE_PESOS_DESCONEXO "graph_teste_pesos_desconexo.txt"

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

static void checar_double(const char *descricao, double obtido, double esperado) {
    char obt[MAX_MSG], esp[MAX_MSG];
    snprintf(obt, MAX_MSG, "%g", obtido);
    snprintf(esp, MAX_MSG, "%g", esperado);
    imprimir_resultado(descricao, obt, fabs(obtido - esperado) < 1e-9, esp);
}

static void checar_bool(const char *descricao, bool obtido, bool esperado) {
    imprimir_resultado(descricao, obtido ? "True" : "False", obtido == esperado,
                       esperado ? "True" : "False");
}

static void checar_string(const char *descricao, const char *obtido, const char *esperado) {
    imprimir_resultado(descricao, obtido, strcmp(obtido, esperado) == 0, esperado);
}

/* "v:peso v:peso" para comparar vizinhos e pesos entre representacoes */
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

/* "[d0, d1, ...]" com %g (infinito aparece como "inf") */
static void dist_para_string(DijkstraResult *r, int n, char *dest) {
    dest[0] = '\0';
    strcat(dest, "[");
    for (int v = 0; v < n; v++) {
        char num[32];
        snprintf(num, sizeof num, (v == 0) ? "%g" : ", %g", r->dist[v]);
        strcat(dest, num);
    }
    strcat(dest, "]");
}

/* "[i0, i1, ...]" para comparar vetores de inteiros (pais, caminhos, niveis) */
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

/* checar_erro do test_parte1: roda a chamada em um processo filho e
   verifica se ela encerra com erro (ValueError -> exit(EXIT_FAILURE)) */
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

static Graph *g_erro = NULL;

static void erro_dijkstra_99(void) { Graph_dijkstra(g_erro, 99, 1); }

static void testar_leitura_com_pesos(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(EXEMPLO_PESOS, adj_list);

    snprintf(descricao, MAX_MSG, "arquivo com pesos: flag weighted [%s]", rep);
    checar_bool(descricao, g->weighted, true);
    snprintf(descricao, MAX_MSG, "peso negativo detectado [%s]", rep);
    checar_bool(descricao, g->negative_weight, true);

    /* as buscas da Parte 1 ignoram os pesos: mesmo grafo subjacente do
       graph_exemplo.txt (as mesmas arestas) */
    GraphSearch *bfs = Graph_BFS(g, 1, 1, 0);
    snprintf(descricao, MAX_MSG, "BFS ignora pesos: niveis [%s]", rep);
    inteiros_para_string(bfs->levels, g->n, obt);
    checar_string(descricao, obt, "[0, 1, 2, 2, 1]");
    GraphSearch_free(bfs);

    /* Dijkstra recusa pesos negativos (devolve NULL, com mensagem), nas
       duas formas e nas duas representacoes */
    for (int modo_i = 0; modo_i <= 1; modo_i++) {
        DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
        snprintf(descricao, MAX_MSG, "dijkstra %s recusa peso negativo [%s]",
                 modo_i ? "heap" : "vetor", rep);
        checar_bool(descricao, dijkstra(g, 0, modo) == NULL, true);
    }

    Graph_destroy(g);
}

static void testar_dijkstra_exemplo(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(EXEMPLO_PESOS_POSITIVOS, adj_list);

    snprintf(descricao, MAX_MSG, "pesos positivos: sem negativos [%s]", rep);
    checar_bool(descricao, g->negative_weight, false);

    /* d(1,2)=0.1; d(1,5)=0.1+0.2=0.3; d(1,4)=0.3+2.3=2.6; d(1,3)=0.3+5=5.3 */
    for (int modo_i = 0; modo_i <= 1; modo_i++) {
        DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
        DijkstraResult *r = dijkstra(g, 0, modo);

        snprintf(descricao, MAX_MSG, "dijkstra %s distancias [%s]",
                 modo_i ? "heap" : "vetor", rep);
        dist_para_string(r, g->n, obt);
        checar_string(descricao, obt, "[0, 0.1, 5.3, 2.6, 0.3]");

        snprintf(descricao, MAX_MSG, "dijkstra %s pais (arvore) [%s]",
                 modo_i ? "heap" : "vetor", rep);
        inteiros_para_string(r->parents, g->n, obt);
        checar_string(descricao, obt, "[-1, 0, 4, 4, 1]");

        /* caminho ate o vertice 4 (rotulo): 1 -> 2 -> 5 -> 4 */
        int comprimento = 0;
        int *caminho = dijkstra_path(r, 3, &comprimento);
        snprintf(descricao, MAX_MSG, "dijkstra %s caminho ate 4 [%s]",
                 modo_i ? "heap" : "vetor", rep);
        inteiros_para_string(caminho, comprimento, obt);
        checar_string(descricao, obt, "[0, 1, 4, 3]");
        checar_int("comprimento do caminho", comprimento, 4);
        free(caminho);

        DijkstraResult_free(r);
    }

    Graph_destroy(g);
}

static void testar_duplicatas_e_peso_tardio(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(TESTE_PESOS, adj_list);

    /* "1 2" (sem peso, vale 1) e "1 2 2.5": vale a ultima linha, 2.5, nas
       duas representacoes. Na matriz isso exercita o "backfill": a matriz
       de pesos nasce na aresta 2.5 e preenche com 1.0 as arestas anteriores.
       O laco "2 2 0.5" e ignorado como na Parte 1. */
    for (int v = 0; v < g->n; v++) {
        snprintf(descricao, MAX_MSG, "vizinhos e pesos do vertice %d [%s]", v + 1, rep);
        vizinhos_pesos_para_string(g, v, obt);
        switch (v) {
            case 0: checar_string(descricao, obt, "1:2.5"); break;
            case 1: checar_string(descricao, obt, "0:2.5 2:1"); break;
            case 2: checar_string(descricao, obt, "1:1 3:1"); break;
            case 3: checar_string(descricao, obt, "2:1 4:1"); break;
            case 4: checar_string(descricao, obt, "3:1"); break;
        }
    }

    checar_int("numero de arestas (duplicatas fundidas)", edge_count(g), 4);

    /* d(1,5) = 2.5 + 1 + 1 + 1 = 5.5 pela cadeia 1-2-3-4-5 */
    DijkstraResult *r = dijkstra(g, 0, DIJKSTRA_VETOR);
    dist_para_string(r, g->n, obt);
    snprintf(descricao, MAX_MSG, "distancias com duplicata em 2.5 [%s]", rep);
    checar_string(descricao, obt, "[0, 2.5, 3.5, 4.5, 5.5]");

    int comprimento = 0;
    int *caminho = dijkstra_path(r, 4, &comprimento);
    inteiros_para_string(caminho, comprimento, obt);
    snprintf(descricao, MAX_MSG, "caminho 1->5 pela cadeia [%s]", rep);
    checar_string(descricao, obt, "[0, 1, 2, 3, 4]");
    free(caminho);
    DijkstraResult_free(r);

    Graph_destroy(g);
}

static void testar_pesos_desconexos(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(TESTE_PESOS_DESCONEXO, adj_list);

    for (int modo_i = 0; modo_i <= 1; modo_i++) {
        DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
        DijkstraResult *r = dijkstra(g, 0, modo);

        snprintf(descricao, MAX_MSG, "dijkstra %s em grafo desconexo [%s]",
                 modo_i ? "heap" : "vetor", rep);
        dist_para_string(r, g->n, obt);
        checar_string(descricao, obt, "[0, 0.5, 1.2, inf, inf, inf, inf]");

        /* caminho dentro da componente e caminho inalcancavel */
        int comprimento = 0;
        int *caminho = dijkstra_path(r, 2, &comprimento);
        snprintf(descricao, MAX_MSG, "caminho ate o vertice 3 [%s]", rep);
        inteiros_para_string(caminho, comprimento, obt);
        checar_string(descricao, obt, "[0, 1, 2]");
        free(caminho);

        caminho = dijkstra_path(r, 3, &comprimento);
        snprintf(descricao, MAX_MSG, "caminho inalcancavel: NULL e 0 [%s]", rep);
        checar_bool(descricao, caminho == NULL && comprimento == 0, true);
        free(caminho);

        DijkstraResult_free(r);
    }

    Graph_destroy(g);
}

static void testar_grafo_sem_pesos(bool adj_list) {
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    char obt[MAX_MSG];
    Graph *g = Graph_create(EXEMPLO, adj_list);

    snprintf(descricao, MAX_MSG, "arquivo sem pesos: weighted false [%s]", rep);
    checar_bool(descricao, g->weighted, false);

    /* toda aresta vale 1: o Dijkstra reproduz os niveis da BFS */
    GraphSearch *bfs = Graph_BFS(g, 1, 1, 0);
    for (int modo_i = 0; modo_i <= 1; modo_i++) {
        DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
        DijkstraResult *r = dijkstra(g, 0, modo);
        snprintf(descricao, MAX_MSG, "sem pesos: dijkstra %s = niveis da BFS [%s]",
                 modo_i ? "heap" : "vetor", rep);
        dist_para_string(r, g->n, obt);
        checar_string(descricao, obt, "[0, 1, 2, 2, 1]");
        DijkstraResult_free(r);
    }
    GraphSearch_free(bfs);
    Graph_destroy(g);
}

static void testar_heap(void) {
    MinHeap heap1;
    MinHeap_init(&heap1, 8);

    MinHeap_insert(&heap1, 5, 10.0);
    MinHeap_insert(&heap1, 3, 20.0);
    MinHeap_insert(&heap1, 7, 30.0);

    checar_bool("heap: contains(3)", MinHeap_contains(&heap1, 3), true);
    checar_bool("heap: contains(1)", MinHeap_contains(&heap1, 1), false);

    MinHeap_decrease_key(&heap1, 3, 5.0); /* 3 passa a frente de 5 */
    checar_int("heap: extract_min apos decrease", MinHeap_extract_min(&heap1), 3);
    checar_bool("heap: contains(3) apos extrair", MinHeap_contains(&heap1, 3), false);
    checar_int("heap: extract_min", MinHeap_extract_min(&heap1), 5);
    checar_int("heap: extract_min", MinHeap_extract_min(&heap1), 7);
    checar_int("heap: extract_min vazio", MinHeap_extract_min(&heap1), -1);
    checar_bool("heap: isEmpty", MinHeap_isEmpty(&heap1), true);

    MinHeap_destroy(&heap1);
}

static void testar_equivalencia_entre_representacoes(void) {
    /* dist/parents iguais nas duas representacoes, nos dois modos, nos
       grafos com pesos (positivos) */
    const char *arquivos[2] = {EXEMPLO_PESOS_POSITIVOS, TESTE_PESOS};
    char descricao[MAX_MSG];
    char obt[MAX_MSG], esp[MAX_MSG];

    for (int f = 0; f < 2; f++) {
        Graph *gm = Graph_create(arquivos[f], false);
        Graph *gl = Graph_create(arquivos[f], true);

        for (int raiz = 1; raiz <= gm->n; raiz++) {
            for (int modo_i = 0; modo_i <= 1; modo_i++) {
                DijkstraModo modo = modo_i ? DIJKSTRA_HEAP : DIJKSTRA_VETOR;
                DijkstraResult *rm = dijkstra(gm, raiz - 1, modo);
                DijkstraResult *rl = dijkstra(gl, raiz - 1, modo);

                snprintf(descricao, MAX_MSG, "dist iguais (%s, raiz %d, %s)",
                         arquivos[f], raiz, modo_i ? "heap" : "vetor");
                dist_para_string(rl, gm->n, obt);
                dist_para_string(rm, gm->n, esp);
                imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);

                snprintf(descricao, MAX_MSG, "pais iguais (%s, raiz %d, %s)",
                         arquivos[f], raiz, modo_i ? "heap" : "vetor");
                inteiros_para_string(rl->parents, gm->n, obt);
                inteiros_para_string(rm->parents, gm->n, esp);
                imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);

                DijkstraResult_free(rm);
                DijkstraResult_free(rl);
            }
        }

        Graph_destroy(gm);
        Graph_destroy(gl);
    }
}

static void testar_equivalencia_entre_modos(void) {
    /* no mesmo grafo, vetor e heap dao exatamente a mesma resposta */
    const char *arquivos[2] = {EXEMPLO_PESOS_POSITIVOS, TESTE_PESOS_DESCONEXO};
    char descricao[MAX_MSG];
    char obt[MAX_MSG], esp[MAX_MSG];

    for (int f = 0; f < 2; f++) {
        Graph *g = Graph_create(arquivos[f], true);

        for (int raiz = 1; raiz <= g->n; raiz++) {
            DijkstraResult *rv = dijkstra(g, raiz - 1, DIJKSTRA_VETOR);
            DijkstraResult *rh = dijkstra(g, raiz - 1, DIJKSTRA_HEAP);

            snprintf(descricao, MAX_MSG, "vetor = heap em dist (%s, raiz %d)",
                     arquivos[f], raiz);
            dist_para_string(rv, g->n, obt);
            dist_para_string(rh, g->n, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);

            snprintf(descricao, MAX_MSG, "vetor = heap em pais (%s, raiz %d)",
                     arquivos[f], raiz);
            inteiros_para_string(rv->parents, g->n, obt);
            inteiros_para_string(rh->parents, g->n, esp);
            imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);

            DijkstraResult_free(rv);
            DijkstraResult_free(rh);
        }

        Graph_destroy(g);
    }
}

static void testar_rotulos_invalidos(void) {
    g_erro = Graph_create(EXEMPLO_PESOS_POSITIVOS, true);
    checar_erro("Graph_dijkstra rejeita rotulo 99", erro_dijkstra_99);
    Graph_destroy(g_erro);
    g_erro = NULL;
}

static void testar_wrapper_graph(void) {
    /* Graph_dijkstra (rotulos 1-based) devolve o mesmo resultado do modulo */
    Graph *g = Graph_create(EXEMPLO_PESOS_POSITIVOS, true);
    DijkstraResult *r = Graph_dijkstra(g, 1, 1);
    char obt[MAX_MSG];

    dist_para_string(r, g->n, obt);
    checar_string("Graph_dijkstra distancias", obt, "[0, 0.1, 5.3, 2.6, 0.3]");

    checar_double("Graph_dijkstra d(1,4)", r->dist[3], 2.6);
    DijkstraResult_free(r);
    Graph_destroy(g);
}

int main(void) {
    const char *representacoes[2] = {"matriz de adjacencia", "lista de adjacencia"};

    for (int i = 0; i < 2; i++) {
        bool adj_list = (i == 1);

        printf("\n=== graph_exemplo_pesos.txt (Figura 1, com -9.5) / %s ===\n", representacoes[i]);
        testar_leitura_com_pesos(adj_list);

        printf("\n=== graph_exemplo_pesos_positivos.txt / %s ===\n", representacoes[i]);
        testar_dijkstra_exemplo(adj_list);

        printf("\n=== graph_teste_pesos.txt (duplicata + peso tardio) / %s ===\n", representacoes[i]);
        testar_duplicatas_e_peso_tardio(adj_list);

        printf("\n=== graph_teste_pesos_desconexo.txt / %s ===\n", representacoes[i]);
        testar_pesos_desconexos(adj_list);

        printf("\n=== graph_exemplo.txt (sem pesos) / %s ===\n", representacoes[i]);
        testar_grafo_sem_pesos(adj_list);
    }

    printf("\n=== heap (decrease-key) ===\n");
    testar_heap();

    printf("\n=== equivalencia entre representacoes ===\n");
    testar_equivalencia_entre_representacoes();

    printf("\n=== equivalencia entre modos (vetor x heap) ===\n");
    testar_equivalencia_entre_modos();

    printf("\n=== rotulos invalidos ===\n");
    testar_rotulos_invalidos();

    printf("\n=== wrapper Graph_dijkstra ===\n");
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
