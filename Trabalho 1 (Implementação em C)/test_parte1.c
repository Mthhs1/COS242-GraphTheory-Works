/*
Testes de verificacao da Parte 1.

Rodar com:  ./test_parte1

Os valores esperados foram conferidos a mao a partir dos dois grafos de teste:

  graph_exemplo.txt          -> o grafo da Figura 1 do enunciado (n=5, m=5)
  graph_teste_desconexo.txt  -> n=7, m=5, tres componentes, um vertice isolado

O segundo grafo existe justamente porque m != n: o leitor original lia n linhas
de aresta, e o exemplo do enunciado (onde m == n) mascarava esse erro.

A API publica usa rotulos 1-based (como no arquivo de entrada); os vetores
devolvidos (ordem, pais, niveis) sao indexados em 0.

Adaptacoes do C: o dict de estatisticas vira o struct GraphStats, as listas de
componentes viram o struct Components, e checar_erro roda a chamada em um
processo filho e verifica se ela encerra com erro (o equivalente do Python e
capturar o ValueError lancado pelo _to_index).
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "graph.h"
#include "graph_utils.h"
#include "graph_stats.h"
#include "components.h"
#include "distance.h"

#define EXEMPLO "graph_exemplo.txt"
#define DESCONEXO "graph_teste_desconexo.txt"

#define MAX_FALHAS 64
#define MAX_MSG 256

static char falhas[MAX_FALHAS][MAX_MSG];
static int falhas_count = 0;

static const char *bool_para_string(bool b) {
    return b ? "True" : "False";
}

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
    imprimir_resultado(descricao, bool_para_string(obtido), obtido == esperado,
                       bool_para_string(esperado));
}

static void checar_string(const char *descricao, const char *obtido, const char *esperado) {
    imprimir_resultado(descricao, obtido, strcmp(obtido, esperado) == 0, esperado);
}

static void array_para_string(const int *v, int len, char *dest) {
    dest[0] = '\0';
    strcat(dest, "[");
    for (int i = 0; i < len; i++) {
        char num[16];
        snprintf(num, sizeof num, (i == 0) ? "%d" : ", %d", v[i]);
        strcat(dest, num);
    }
    strcat(dest, "]");
}

static void checar_array(const char *descricao, const int *obtido, const int *esperado, int len) {
    char obt[MAX_MSG], esp[MAX_MSG];
    array_para_string(obtido, len, obt);
    array_para_string(esperado, len, esp);
    bool ok = (len <= 0) || (obtido != NULL && esperado != NULL &&
                             memcmp(obtido, esperado, (size_t)len * sizeof(int)) == 0);
    imprimir_resultado(descricao, obt, ok, esp);
}

static void components_para_string(Components *c, char *dest) {
    dest[0] = '\0';
    strcat(dest, "[");
    for (int i = 0; i < c->count; i++) {
        if (i > 0) {
            strcat(dest, ", ");
        }
        strcat(dest, "[");
        for (int j = 0; j < c->sizes[i]; j++) {
            char num[16];
            snprintf(num, sizeof num, (j == 0) ? "%d" : ", %d", c->components[i][j]);
            strcat(dest, num);
        }
        strcat(dest, "]");
    }
    strcat(dest, "]");
}

static double round4(double x) {
    return round(x * 10000.0) / 10000.0;
}

static int comparar_ascendente(const void *x, const void *y) {
    int a = *(const int *)x;
    int b = *(const int *)y;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* checar_erro do Python: aqui a funcao roda em um processo filho e o teste
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

static void erro_BFS_0(void) { Graph_BFS(g_erro, 0, 0, 0); }
static void erro_BFS_6(void) { Graph_BFS(g_erro, 6, 0, 0); }
static void erro_BFS_menos1(void) { Graph_BFS(g_erro, -1, 0, 0); }
static void erro_DFS_6(void) { Graph_DFS(g_erro, 6, 0); }
static void erro_distance_6(void) { Graph_distance(g_erro, 1, 6); }
static void erro_target_6(void) { Graph_BFS(g_erro, 1, 0, 6); }

static void testar_exemplo(bool adj_list) {
    Graph *g = Graph_create(EXEMPLO, adj_list);

    int esperado_graus[5] = {2, 2, 1, 1, 4};
    int *graus = all_degrees(g);
    checar_array("graus", graus, esperado_graus, 5);
    free(graus);

    checar_int("numero de arestas", edge_count(g), 5);

    GraphStats s = graph_stats(g);
    checar_int("grau minimo", s.grau_minimo, 1);
    checar_int("grau maximo", s.grau_maximo, 4);
    checar_double("grau medio", round4(s.grau_medio), 2.0);
    checar_double("mediana de grau", s.mediana_grau, 2.0);

    Components comps = connected_components(g);
    char obt[MAX_MSG];
    components_para_string(&comps, obt);
    checar_string("componentes", obt, "[[1, 2, 3, 4, 5]]");

    checar_int("d(1,2)", distance(g, 1, 2, true), 1);
    checar_int("d(1,3)", distance(g, 1, 3, true), 2);
    checar_int("d(3,4)", distance(g, 3, 4, true), 2);
    checar_int("diametro", diameter(g), 2);
    checar_int("diametro aproximado", approximate_diameter(g), 2);

    Components_free(&comps);
    Graph_destroy(g);
}

static void testar_desconexo(bool adj_list) {
    Graph *g = Graph_create(DESCONEXO, adj_list);

    int esperado_graus[7] = {2, 2, 2, 0, 1, 2, 1};
    int *graus = all_degrees(g);
    checar_array("graus", graus, esperado_graus, 7);
    free(graus);

    checar_int("numero de arestas", edge_count(g), 5);

    GraphStats s = graph_stats(g);
    checar_int("grau minimo", s.grau_minimo, 0);
    checar_int("grau maximo", s.grau_maximo, 2);
    checar_double("grau medio", round4(s.grau_medio), round4(10.0 / 7.0));
    checar_double("mediana de grau", s.mediana_grau, 2.0);

    Components comps = connected_components(g);
    checar_int("numero de componentes", comps.count, 3);

    char obt[MAX_MSG];
    array_para_string(comps.sizes, comps.count, obt);
    checar_string("tamanhos (ordem decrescente)", obt, "[3, 3, 1]");
    array_para_string(comps.components[comps.count - 1], comps.sizes[comps.count - 1], obt);
    checar_string("menor componente", obt, "[4]");

    checar_int("d(1,3) mesma componente", distance(g, 1, 3, true), 1);
    checar_int("d(5,7) mesma componente", distance(g, 5, 7, true), 2);
    checar_int("d(1,5) componentes distintas", distance(g, 1, 5, true), INFINITO);
    checar_int("diametro", diameter(g), 2);

    Components_free(&comps);
    Graph_destroy(g);
}

static void testar_mediana(void) {
    int impar[3] = {1, 3, 2};
    checar_double("mediana impar", median(impar, 3), 2.0);
    int par[4] = {1, 2, 3, 4};
    checar_double("mediana par", median(par, 4), 2.5);
    checar_double("mediana vazia", median(NULL, 0), 0.0);
}

static void testar_arvore_bfs(void) {
    /* A BFS a partir do vertice 5 (rotulo 1-based, indice 4) alcanca todos em um nivel. */
    Graph *g = Graph_create(EXEMPLO, true);
    GraphSearch *busca = Graph_BFS(g, 5, 1, 0);

    checar_int("nivel da raiz", busca->levels[4], 0);

    int obtido[4];
    int esperado_niveis[4] = {1, 1, 1, 1};
    for (int i = 0; i < 4; i++) {
        obtido[i] = busca->levels[i];
    }
    checar_array("niveis dos vizinhos de 5", obtido, esperado_niveis, 4);

    int esperado_pais[4] = {4, 4, 4, 4};
    for (int i = 0; i < 4; i++) {
        obtido[i] = busca->parents[i];
    }
    checar_array("pais dos vizinhos de 5", obtido, esperado_pais, 4);

    GraphSearch_free(busca);
    Graph_destroy(g);
}

static void testar_contrato_de_retorno(bool adj_list) {
    /* Sem generate_tree a Graph devolve so a ordem; com ele, a tripla completa. */
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    Graph *g = Graph_create(EXEMPLO, adj_list);

    GraphSearch *so_bfs = Graph_BFS(g, 1, 0, 0);
    snprintf(descricao, MAX_MSG, "BFS devolve lista [%s]", rep);
    checar_bool(descricao, so_bfs->parents == NULL && so_bfs->levels == NULL, true);
    GraphSearch_free(so_bfs);

    GraphSearch *so_dfs = Graph_DFS(g, 1, 0);
    snprintf(descricao, MAX_MSG, "DFS devolve lista [%s]", rep);
    checar_bool(descricao, so_dfs->parents == NULL && so_dfs->levels == NULL, true);
    GraphSearch_free(so_dfs);

    GraphSearch *com_bfs = Graph_BFS(g, 1, 1, 0);
    snprintf(descricao, MAX_MSG, "BFS com arvore devolve tripla [%s]", rep);
    checar_bool(descricao, com_bfs->parents != NULL && com_bfs->levels != NULL, true);
    GraphSearch_free(com_bfs);

    GraphSearch *com_dfs = Graph_DFS(g, 1, 1);
    snprintf(descricao, MAX_MSG, "DFS com arvore devolve tripla [%s]", rep);
    checar_bool(descricao, com_dfs->parents != NULL && com_dfs->levels != NULL, true);
    GraphSearch_free(com_dfs);

    Graph_destroy(g);
}

static void testar_target(bool adj_list) {
    /* BFS com target encerra na primeira visita ao alvo, nas duas representacoes. */
    const char *rep = adj_list ? "lista" : "matriz";
    char descricao[MAX_MSG];
    Graph *g = Graph_create(EXEMPLO, adj_list);

    GraphSearch *busca = Graph_BFS(g, 1, 1, 3);
    snprintf(descricao, MAX_MSG, "ultimo visitado e o target [%s]", rep);
    checar_int(descricao, busca->result_discovered[busca->count - 1], 2);
    snprintf(descricao, MAX_MSG, "nivel do target = d(1,3) [%s]", rep);
    checar_int(descricao, busca->levels[2], 2);
    snprintf(descricao, MAX_MSG, "pai do target definido [%s]", rep);
    checar_bool(descricao, busca->parents[2] != -1, true);
    GraphSearch_free(busca);

    GraphSearch *so_ordem = Graph_BFS(g, 1, 0, 3);
    snprintf(descricao, MAX_MSG, "target sem arvore devolve lista [%s]", rep);
    checar_bool(descricao, so_ordem->parents == NULL, true);
    snprintf(descricao, MAX_MSG, "lista termina no target [%s]", rep);
    checar_int(descricao, so_ordem->result_discovered[so_ordem->count - 1], 2);
    GraphSearch_free(so_ordem);

    /* alvo inalcancavel: a busca percorre a componente inteira */
    Graph *d = Graph_create(DESCONEXO, adj_list);
    GraphSearch *ordem_d = Graph_BFS(d, 1, 0, 7);

    int copia[16];
    for (int i = 0; i < ordem_d->count && i < 16; i++) {
        copia[i] = ordem_d->result_discovered[i];
    }
    qsort(copia, (size_t)ordem_d->count, sizeof(int), comparar_ascendente);

    int esperado[3] = {0, 1, 2};
    snprintf(descricao, MAX_MSG, "alvo inalcancavel: componente completa [%s]", rep);
    bool completa = ordem_d->count == 3;
    if (completa) {
        completa = memcmp(copia, esperado, 3 * sizeof(int)) == 0;
    }
    char obt[MAX_MSG];
    array_para_string(copia, ordem_d->count, obt);
    imprimir_resultado(descricao, obt, completa, "[0, 1, 2]");

    bool achou = false;
    for (int i = 0; i < ordem_d->count; i++) {
        if (ordem_d->result_discovered[i] == 6) {
            achou = true;
        }
    }
    snprintf(descricao, MAX_MSG, "alvo inalcancavel: target fora da ordem [%s]", rep);
    checar_bool(descricao, achou, false);

    GraphSearch_free(ordem_d);
    Graph_destroy(d);
    Graph_destroy(g);
}

static void testar_rotulos_invalidos(void) {
    g_erro = Graph_create(EXEMPLO, false);

    checar_erro("BFS rejeita rotulo 0", erro_BFS_0);
    checar_erro("BFS rejeita rotulo 6", erro_BFS_6);
    checar_erro("BFS rejeita rotulo -1", erro_BFS_menos1);
    checar_erro("DFS rejeita rotulo 6", erro_DFS_6);
    checar_erro("distance rejeita vertice 6", erro_distance_6);
    checar_erro("BFS rejeita target 6", erro_target_6);

    Graph_destroy(g_erro);
    g_erro = NULL;
}

static void testar_equivalencia_entre_representacoes(void) {
    /* DFS gera a mesma arvore nas duas representacoes; na BFS, os niveis coincidem. */
    Graph *gm = Graph_create(EXEMPLO, false);
    Graph *gl = Graph_create(EXEMPLO, true);
    char descricao[MAX_MSG];
    char obt[MAX_MSG], esp[MAX_MSG];

    for (int raiz = 1; raiz <= 5; raiz++) {
        GraphSearch *dfs_m = Graph_DFS(gm, raiz, 1);
        GraphSearch *dfs_l = Graph_DFS(gl, raiz, 1);

        snprintf(descricao, MAX_MSG, "DFS ordem igual (raiz %d)", raiz);
        array_para_string(dfs_l->result_discovered, dfs_l->count, obt);
        array_para_string(dfs_m->result_discovered, dfs_m->count, esp);
        bool ok = dfs_l->count == dfs_m->count && strcmp(obt, esp) == 0;
        imprimir_resultado(descricao, obt, ok, esp);

        snprintf(descricao, MAX_MSG, "DFS pais iguais (raiz %d)", raiz);
        array_para_string(dfs_l->parents, gm->n, obt);
        array_para_string(dfs_m->parents, gm->n, esp);
        imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);

        GraphSearch_free(dfs_m);
        GraphSearch_free(dfs_l);

        GraphSearch *bfs_m = Graph_BFS(gm, raiz, 1, 0);
        GraphSearch *bfs_l = Graph_BFS(gl, raiz, 1, 0);

        snprintf(descricao, MAX_MSG, "BFS niveis iguais (raiz %d)", raiz);
        array_para_string(bfs_l->levels, gm->n, obt);
        array_para_string(bfs_m->levels, gm->n, esp);
        imprimir_resultado(descricao, obt, strcmp(obt, esp) == 0, esp);

        GraphSearch_free(bfs_m);
        GraphSearch_free(bfs_l);
    }

    Graph_destroy(gm);
    Graph_destroy(gl);
}

static void testar_write_search_tree(void) {
    /* Requisito 4: arquivo com raiz, pai e nivel de cada vertice (rotulos 1-based). */
    const char *caminho = "arvore_teste.txt";
    Graph *g = Graph_create(EXEMPLO, true);

    char *content = Graph_write_search_tree(g, caminho, 5, 0);
    free(content);

    FILE *file = fopen(caminho, "r");
    if (file == NULL) {
        checar_bool("arquivo da arvore criado", false, true);
        Graph_destroy(g);
        return;
    }

    fseek(file, 0, SEEK_END);
    long tamanho = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = xmalloc((size_t)tamanho + 1);
    size_t lido = fread(buffer, 1, (size_t)tamanho, file);
    buffer[lido] = '\0';
    fclose(file);
    remove(caminho);

    /* splitlines() do Python */
    char *linhas[32];
    int total = 0;
    char *linha = buffer;
    while (*linha != '\0' && total < 32) {
        linhas[total] = linha;
        total += 1;
        char *fim = strchr(linha, '\n');
        if (fim == NULL) {
            break;
        }
        *fim = '\0';
        linha = fim + 1;
    }

    checar_string("raiz registrada", linhas[0], "Raiz da busca: 5");
    checar_string("linha do vertice 1", linhas[3], "1 5 1");
    checar_string("linha da raiz", linhas[7], "5 - 0");
    checar_int("total de vertices na arvore", total - 3, 5);

    free(buffer);
    Graph_destroy(g);
}

int main(void) {
    const char *representacoes[2] = {"matriz de adjacencia", "lista de adjacencia"};

    for (int i = 0; i < 2; i++) {
        bool adj_list = (i == 1);

        printf("\n=== graph_exemplo.txt / %s ===\n", representacoes[i]);
        testar_exemplo(adj_list);

        printf("\n=== graph_teste_desconexo.txt / %s ===\n", representacoes[i]);
        testar_desconexo(adj_list);

        printf("\n=== contrato de retorno / %s ===\n", representacoes[i]);
        testar_contrato_de_retorno(adj_list);

        printf("\n=== BFS com target / %s ===\n", representacoes[i]);
        testar_target(adj_list);
    }

    printf("\n=== mediana ===\n");
    testar_mediana();

    printf("\n=== arvore da BFS ===\n");
    testar_arvore_bfs();

    printf("\n=== rotulos invalidos ===\n");
    testar_rotulos_invalidos();

    printf("\n=== equivalencia entre representacoes ===\n");
    testar_equivalencia_entre_representacoes();

    printf("\n=== arquivo da arvore de busca ===\n");
    testar_write_search_tree();

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
