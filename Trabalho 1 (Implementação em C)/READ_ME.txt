================================================================================
READ_ME - Biblioteca de Grafos em C (Trabalho 1, COS 242)
================================================================================

Biblioteca para manipular grafos NAO-DIRECIONADOS, com as duas
representacoes exigidas pelo enunciado (matriz e lista de adjacencia) e os
algoritmos da Parte 1: BFS, DFS, distancia, diametro (exato e aproximado),
componentes conexas e estatisticas do grafo em arquivo.


--------------------------------------------------------------------------------
1. FORMATO DO ARQUIVO DE ENTRADA (graph.txt)
--------------------------------------------------------------------------------
A primeira linha informa o NUMERO DE VERTICES do grafo (n). Cada linha
seguinte informa UMA aresta, com os rotulos dos dois vertices separados por
espaco. Os rotulos sao 1-based (o primeiro vertice e o 1, o ultimo e o n).

    5          <- numero de vertices
    1 2        <- aresta entre os vertices 1 e 2
    2 5
    5 3
    4 5
    1 5

Este e exatamente o exemplo da Figura 1 do enunciado (veja
graph_exemplo.txt nesta pasta).

Observacoes sobre o formato:
- O numero de arestas (m) NAO precisa ser igual ao numero de vertices.
- Como o grafo e nao-direcionado, "a b" cria a aresta nos dois sentidos
  (nao repita "b a" no arquivo).
- Vertices isolados simplesmente nao aparecem em nenhuma linha de aresta.
- Linhas em branco sao ignoradas.
- Evite arestas duplicadas no arquivo: a representacao em matriz as
  funde em uma so, mas a lista de adjacencia as guardaria duplicadas.


--------------------------------------------------------------------------------
2. ESTRUTURA DAS PASTAS E COMPILACAO
--------------------------------------------------------------------------------
Organizacao dos fontes:

    DataStructures/   estruturas de dados basicas: no (node), fila
                      (my_queue) e pilha (my_stack)
    Algorithms/       algoritmos sobre o grafo: buscas (BFS, DFS),
                      componentes conexas, distancia/diametro (distance)
                      e estatisticas (graph_stats)
    Study cases/      programas da secao 4 do enunciado (estudos de caso)
    bin/              executaveis gerados pelo make (criada ao compilar)
    raiz              a "classe" Graph (graph.c/h), a leitura do arquivo
                      de entrada (graph_gen), utilitarios compartilhados
                      (graph_utils), os testes (test_parte1.c), os grafos
                      de exemplo (.txt) e os guias (READ_ME, ADAPTACOES)

Requisitos: gcc e make.

    make                 -> compila os testes (bin/test_parte1) e o demo (bin/graph)
    make test            -> compila e roda os testes
    make clean           -> remove a pasta bin/ com os executaveis

Outros alvos (constroem, se precisar, e RODAM o programa - veja secao 5):
    make graph  make my_queue_demo  make my_stack_demo
    make components_demo  make distance_demo  make graph_stats_demo

Estudos de caso (secao 4 do enunciado - veja secao 6):
    make casos           -> compila os 6 programas das questoes (bin/questao*)
    make questao1  make questao2_3  make questao4
    make questao5  make questao6  make questao7

Os executaveis ficam em bin/, mas devem rodar A PARTIR DESTA PASTA (a
raiz), pois usam caminhos relativos (por exemplo, o demo "graph" le
"../graph1.txt", que fica na raiz do repositorio - o mesmo arquivo que o
Python le).


--------------------------------------------------------------------------------
3. COMO CARREGAR UM SEU graph.txt NO PROGRAMA
--------------------------------------------------------------------------------
Ha duas formas.

(a) RAPIDA, usando os demos: os demos trazem o caminho do arquivo fixo no
    codigo. Copie o seu arquivo para a pasta com um desses nomes:

        cp meu_grafo.txt graph_exemplo.txt     # demos components/distance/graph_stats
        cp meu_grafo.txt ../graph1.txt         # demo "graph" (./bin/graph)

    ou ajuste a constante no inicio do main correspondente (por exemplo,
    GRAPH_MAIN no fim de graph.c) e recompile com make.

(b) RECOMENDADA, usando a biblioteca no seu proprio programa: crie um
    arquivo .c que inclua graph.h e chame Graph_create com o caminho do
    seu arquivo. A funcao recebe a representacao escolhida:

        Graph *g = Graph_create("meu_grafo.txt", false);  // matriz
        Graph *g = Graph_create("meu_grafo.txt", true);   // lista

    Exemplo completo (salve como meu_programa.c nesta pasta):

        #include <stdio.h>
        #include <stdlib.h>

        #include "graph.h"
        #include "graph_stats.h"  /* struct GraphStats    */
        #include "components.h"   /* struct Components    */

        int main(void) {
            /* 1. Carrega o grafo: false = matriz, true = lista (requisito 3). */
            Graph *g = Graph_create("meu_grafo.txt", false);

            /* 2. Estatisticas (requisito 2) + arquivo de saida. */
            GraphStats *s = Graph_stats(g);
            printf("vertices: %d\narestas: %d\ngrau maximo: %d\n",
                   s->vertices, s->arestas, s->grau_maximo);
            free(s);

            char *content = Graph_write_output(g, "saida.txt");
            free(content);

            /* 3. Buscas com arvore (requisito 4); rotulos 1-based. */
            GraphSearch *busca = Graph_BFS(g, 1, 1, 0);
            printf("nivel do vertice 3 na BFS a partir de 1: %d\n",
                   busca->levels[3 - 1]);
            GraphSearch_free(busca);

            Graph_write_search_tree(g, "arvore_bfs.txt", 1, 0);

            /* 4. Distancia e diametro (requisito 5). */
            printf("d(1,3) = %d\n", Graph_distance(g, 1, 3));
            printf("diametro exato     = %d\n", Graph_diameter(g, 0));
            printf("diametro aproximado = %d\n", Graph_diameter(g, 1));

            /* 5. Componentes conexas (requisito 6), ordem decrescente. */
            Components *comps = Graph_connected_components(g);
            printf("%d componentes; a maior tem %d vertices\n",
                   comps->count, comps->sizes[0]);
            Components_free(comps);

            /* 6. Em C nao ha coletor de lixo: libere o que alocou. */
            Graph_destroy(g);
            return 0;
        }

    Compile linkando com todos os modulos da biblioteca (as flags -I dizem
    ao compilador onde estao os cabecalhos das pastas):

        gcc -std=c11 -Wall -Wextra -I. -IDataStructures -IAlgorithms \
            -o meu_programa meu_programa.c \
            DataStructures/node.c DataStructures/my_queue.c \
            DataStructures/my_stack.c graph_gen.c graph_utils.c \
            Algorithms/BFS.c Algorithms/DFS.c Algorithms/components.c \
            Algorithms/distance.c Algorithms/graph_stats.c graph.c -lm


--------------------------------------------------------------------------------
4. RESUMO DA API (orientada a quem vai usar a biblioteca)
--------------------------------------------------------------------------------
Rotulos de vertices na API publica sao SEMPRE 1-based (como no arquivo de
entrada). Os vetores devolvidos (ordem, pais, niveis) sao indexados em 0:
busca->levels[v - 1] e o nivel do vertice v. Em parents, -1 equivale ao
None do Python (sem pai / nao alcancado).

    Graph *Graph_create(const char *arquivo, bool adj_list);
        Carrega o grafo do arquivo. adj_list: true = lista de adjacencia,
        false = matriz de adjacencia.

    void Graph_destroy(Graph *g);
        Libera o grafo (chame ao terminar).

    char *Graph_str(Graph *g);
        Representacao textual (a "lista" ou a matriz); libere com free().

    GraphSearch *Graph_BFS(Graph *g, int inicio, int generate_tree, int target);
    GraphSearch *Graph_DFS(Graph *g, int inicio, int generate_tree);
        Busca a partir do vertice `inicio` (1-based). Com generate_tree == 0
        devolvem apenas a ordem (parents/levels ficam NULL); com 1, devolvem
        a tripla completa. `target` (1-based) encerra a BFS ao alcancar o
        vertice; use 0 para nao ter alvo. Libere com GraphSearch_free().

    int Graph_distance(Graph *g, int u, int v);
        Distancia entre u e v; -1 se estiverem em componentes distintas.

    int Graph_diameter(Graph *g, int aproximado);
        Diametro exato (0) ou aproximado por dupla varredura (1) - use o
        aproximado em grafos muito grandes.

    Components *Graph_connected_components(Graph *g);
        Componentes em ordem DECRESCENTE de tamanho; rotulos 1-based.
        Libere com Components_free().

    GraphStats *Graph_stats(Graph *g);
        n, m, grau minimo/maximo/medio e mediana de grau. Libere com free().

    char *Graph_write_output(Graph *g, const char *caminho);
        Escreve o arquivo de saida do requisito 2 (estatisticas +
        componentes). Retorna o conteudo escrito; libere com free().

    char *Graph_write_search_tree(Graph *g, const char *caminho,
                                  int inicio, int use_dfs);
        Escreve o arquivo do requisito 4: raiz, pai e nivel de cada vertice
        da arvore de busca (use_dfs: 0 = BFS, 1 = DFS).


--------------------------------------------------------------------------------
5. PROGRAMAS INCLUIDOS
--------------------------------------------------------------------------------
    ./bin/test_parte1      Testes da Parte 1 (74 verificacoes). Entrada:
                           graph_exemplo.txt e graph_teste_desconexo.txt.
    ./bin/graph            Demo da classe Graph; le ../graph1.txt.
    ./bin/my_queue_demo    Demo da fila (my_queue.py).
    ./bin/my_stack_demo    Demo da pilha (my_stack.py).
    ./bin/components_demo  Demo das componentes; le graph_exemplo.txt.
    ./bin/distance_demo    Demo de distancias/diametro; le graph_exemplo.txt.
    ./bin/graph_stats_demo Demo do requisito 2; gera saida_exemplo.txt.

Programas dos estudos de caso (secao 6; fontes na pasta "Study cases"):
    ./bin/questao1         Q1: memoria das duas representacoes (pausa manual).
    ./bin/questao2_3       Q2/Q3: tempo medio de 100 BFS e 100 DFS.
    ./bin/questao4         Q4: pais de 10/20/30 nas arvores BFS/DFS (raizes 1/2/3).
    ./bin/questao5         Q5: distancias (10,20), (10,30), (20,30).
    ./bin/questao6         Q6: componentes conexas (numero, maior, menor).
    ./bin/questao7         Q7: diametro exato e/ou aproximado.

Atalho: `make <nome>` tambem constroi (se precisar) e roda cada programa -
por exemplo, `make graph` roda o demo e `make test_parte1` roda os testes
(o mesmo que `make test`).

A lista completa das adaptacoes feitas na conversao Python -> C esta no
arquivo ADAPTACOES_C.txt, nesta mesma pasta.


--------------------------------------------------------------------------------
6. ESTUDOS DE CASO (secao 4 do enunciado)
--------------------------------------------------------------------------------
Programas dedicados, na pasta "Study cases", um arquivo por questao. Os
demos da biblioteca NAO servem para os grafos do estudo de caso (imprimem a
representacao inteira; alem disso, a matriz de um grafo com ~50 mil
vertices ocupa ~10GB).

Resumo rapido (todos recebem <grafo.txt> <matriz|lista> e rodem da raiz):

    questao  o que faz                                        fonte (Study cases/)
    1        memoria: pausa "Measure memory." apos carregar      questao1_memoria.c
    2 e 3    tempo medio de 100 BFS e 100 DFS                   questao2_3_buscas.c
    4        pais de 10/20/30 nas arvores BFS/DFS (raizes 1-3)  questao4_pais.c
    5        distancias (10,20), (10,30), (20,30)               questao5_distancias.c
    6        componentes conexas: numero, maior, menor           questao6_componentes.c
    7        diametro exato e/ou aproximado                      questao7_diametro.c

Compilar todos de uma vez:

    make casos           (ou make questao1, make questao2_3, ... individualmente)

Exemplos com o grafo do estudo de caso (fica na raiz do repositorio):

    ./bin/questao1 ../graph123.txt lista
        Q1: carrega o grafo e pausa ("Measure memory.") para a medicao
        manual de memoria em outra janela (top/pmap); ENTER encerra. Rode
        uma vez com "lista" e outra com "matriz" para comparar as duas.

    ./bin/questao2_3 ../graph123.txt lista [n_buscas]
        Q2/Q3: n_buscas (padrao 100) buscas BFS e DFS partindo de vertices
        distintos; imprime apenas o tempo medio/min/max de cada busca
        (o cronometro cobre so o algoritmo, como pede o enunciado).

    ./bin/questao4 ../graph123.txt lista
        Q4: pai dos vertices 10, 20 e 30 nas arvores BFS e DFS partindo
        dos vertices 1, 2 e 3 ('-' = fora da componente da raiz). Os pais
        da BFS podem diferir entre matriz e lista (arvores distintas);
        os da DFS sao iguais nas duas representacoes.

    ./bin/questao5 ../graph123.txt lista
        Q5: distancias d(10,20), d(10,30) e d(20,30). -1 indica que o
        par esta em componentes distintas.

    ./bin/questao6 ../graph123.txt lista
        Q6: numero de componentes conexas, tamanho da maior e da menor.

    ./bin/questao7 ../graph123.txt lista [exato|aproximado|ambos]
        Q7: diametro. "aproximado" (padrao) usa a dupla varredura - rapido
        e cota inferior; "exato" faz BFS de cada vertice (so para grafos
        pequenos/medios); "ambos" roda os dois.

Atencao: para grafos grandes, a representacao em matriz pode nao caber em
memoria (n^2 inteiros). Use-a apenas onde for viavel - a comparacao de
memoria e justamente o objeto da Questao 1.
