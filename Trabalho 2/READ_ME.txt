================================================================================
READ_ME - Parte 2 em C: grafos com pesos e Dijkstra (Trabalho 2, COS 242)
================================================================================

Esta pasta e a entrega da Parte 2 (trabalho-P2.pdf): a biblioteca da Parte 1
estendida com PESOS nas arestas e com distancia e caminho minimo por DIJKSTRA,
de duas formas (vetor e heap) - implementado o algoritmo uma unica vez, sobre
a abstracao de como as estimativas sao guardadas e o minimo extraido, como
pede o enunciado.

Os fontes da biblioteca (graph, graph_gen, graph_utils, DataStructures/ e
Algorithms/, menos heap.c e dijkstra.c) sao IDENTICOS aos da pasta
"Trabalho 1 (Implementação em C)": a Parte 2 estendeu a mesma biblioteca - o
no ganhou o campo do peso, a matriz ganhou uma matriz de pesos separada, o
iterador de vizinhanca passou a entregar (vizinho, peso) e foram acrescentados
o heap e o dijkstra. A diferenca entre as pastas:

- o Makefile: aqui so os alvos da Parte 2 (testes, demos e estudos de caso
  novos); os alvos da Parte 1 ficam na pasta dela;
- os grafos de teste da Parte 1 (graph_teste_desconexo.txt e
  graph_teste_lacos_duplicatas.txt) e o test_parte1.c ficam na pasta dela;
  os desta pasta sao os com pesos (e o graph_exemplo.txt, que o test_parte2
  usa no teste "grafo sem pesos");
- a automacao dos estudos tambem foi dividida: a da Parte 1
  (rodar_estudos.sh, questoes 1 a 7) fica na pasta dela; a da Parte 2 fica
  AQUI - Study cases/rodar_estudos_p2.sh, que so roda em grafos COM PESOS
  (tres colunas) e monta resultados/tabelas.md com o gerar_tabelas_p2.py
  (alvo `make estudos`; a questao da rede de colaboracao roda pelas
  variaveis REDE_COLABORACAO e REDE_NOMES, e o Dijkstra com vetor so roda
  em grafos com ate DIJKSTRA_VETOR_MAX vertices, por ser O(n^2)).


--------------------------------------------------------------------------------
1. ESTRUTURA DAS PASTAS E COMPILACAO
--------------------------------------------------------------------------------
    DataStructures/   no da lista (com o peso da aresta), fila, pilha e o NOVO
                      heap binario de minimos com decrease-key (heap.c)
    graph_utils.h     o iterador de vizinhanca (NeighborIter): a unica parte
                      do codigo que sabe a diferenca entre a lista e a matriz
                      e onde moram os pesos - entrega (vizinho, peso)
    Algorithms/       buscas, componentes, distancia/diametro e estatisticas
                      (da Parte 1) e o NOVO dijkstra.c: o algoritmo escrito
                      uma vez, com as estimativas em vetor ou em heap
    Study cases/      programas da secao 3 do enunciado da Parte 2
    bin/              executaveis gerados pelo make (criada ao compilar)
    raiz              a "classe" Graph, a leitura do arquivo (com a terceira
                      coluna de pesos), utilitarios, test_parte2.c, os
                      grafos de exemplo (.txt) e este guia

Requisitos: gcc e make.

    make              -> compila os testes da Parte 2 e os demos novos
    make test         -> compila e roda os testes da Parte 2
    make casos_p2     -> compila os programas dos estudos de caso
    make estudos      -> roda os estudos de caso da Parte 2 nos grafos COM
                         PESOS de ../Grafos e gera resultados/tabelas.md
                         (a questao da rede de colaboracao roda quando as
                         variaveis REDE_COLABORACAO e REDE_NOMES apontam
                         para os arquivos; grafos de duas colunas sao os
                         da Parte 1 e ficam com o `make estudos` de la)
    make clean        -> remove a pasta bin/

Demos (constroem e rodam):
    make heap_demo      insert/decrease_key/extract_min do heap
    make dijkstra_demo  distancias e caminhos com vetor e com heap

Programas (constroem e rodam; detalhes na secao 3):
    make p2_questao1  make p2_questao2  make p2_questao3

Os executaveis ficam em bin/ e rodam A PARTIR DESTA PASTA (caminhos relativos;
os grafos dos estudos de caso ficam em ../Grafos, na raiz do repositorio).


--------------------------------------------------------------------------------
2. FORMATO DO ARQUIVO DE ENTRADA
--------------------------------------------------------------------------------
A primeira linha e o numero de vertices; cada linha seguinte e uma aresta
"a b" ou "a b peso" - o peso e um numero em ponto flutuante, podendo ser
negativo (a leitura aceita; o Dijkstra e que recusa). Sem a terceira coluna,
a aresta vale 1 (os arquivos da Parte 1 continuam validos). Lacos e arestas
repetidas sao ignorados/fundidos; entre repetidas com pesos diferentes, vale
a ultima linha do arquivo, nas duas representacoes.

Exemplo (Figura 1 do enunciado da Parte 2; graph_exemplo_pesos.txt):

    5
    1 2 0.1
    2 5 0.2
    5 3 5
    3 4 -9.5
    4 5 2.3
    1 5 1


--------------------------------------------------------------------------------
3. O QUE A PARTE 2 ACRESCENTOU (E COMO USAR)
--------------------------------------------------------------------------------
Onde ficam os pesos: no no da lista (Node.weight) e, na matriz, numa matriz
double separada, alocada so se o arquivo tem algum peso != 1. O acesso e
sempre pelo iterador de vizinhanca: neighbor_next entrega (vizinho, peso).

API (rotulos 1-based; vetores devolvidos indexados em 0):

    DijkstraResult *Graph_dijkstra(Graph *g, int inicio, int use_heap);
    DijkstraResult *dijkstra(Graph *g, int source, DijkstraModo modo);
        Distancias minimas e arvore de caminhos minimos de `source` para
        todos os vertices, com DIJKSTRA_VETOR (O(n^2)) ou DIJKSTRA_HEAP
        (O(m log n)). Devolve NULL, com mensagem, se o grafo tem aresta com
        peso negativo (a biblioteca ainda nao implementa caminhos minimos
        com pesos negativos - o enunciado manda informar). Libere com
        DijkstraResult_free().

    DijkstraResult: { double *dist; int *parents; }
        dist[v] = INFINITO_PESO (infinito) se v nao e alcancavel; parents e
        a arvore de caminhos minimos (-1 = raiz ou inalcancavel).

    int *dijkstra_path(DijkstraResult *r, int alvo, int *comprimento);
        Caminho minimo fonte -> alvo (0-based); NULL e 0 se inalcancavel.

O algoritmo e escrito uma vez (Algorithms/dijkstra.c) sobre o DistStore -
a abstracao de guardar/atualizar as estimativas e extrair o minimo. As duas
implementacoes plugam nele: vetor (minimo por varredura, O(n^2)) e heap
(decrease-key em O(log n) pelo posicao[vertice] do MinHeap - a "chave
modificada de forma eficiente" que o enunciado exige).

Testes: ./bin/test_parte2 - leitura da terceira coluna nas duas
representacoes, duplicatas com pesos diferentes (vale a ultima), Dijkstra com
vetor = com heap = nas duas representacoes (dist e pais), caminhos,
inalcancaveis, recusa de pesos negativos e, em grafo sem pesos,
dist = niveis da BFS. Os 176 testes da Parte 1 estao na pasta dela e
continuam passando com a biblioteca estendida.

Estudos de caso (secao 3 do enunciado da Parte 2; grafos em ../Grafos):

    ./bin/p2_questao1 ../Grafos/grafo_1.txt lista [vetor|heap]
        Distancia e caminho minimo do vertice 10 aos vertices 20, 30, 40,
        50 e 60. Modo padrao: heap.

    ./bin/p2_questao2 ../Grafos/grafo_1.txt lista [k] [vetor|heap|ambos]
        Tempo medio de k (padrao 100) Dijkstras partindo de vertices
        aleatorios distintos (semente fixa: reproduzivel), com estimativas
        em vetor e/ou em heap. O cronometro cobre so o algoritmo.

    ./bin/p2_questao3 ../Grafos/rede.txt ../Grafos/nomes.txt [vetor|heap]
        Rede de colaboracao: distancia e caminho minimo entre Edsger W.
        Dijkstra e Alan M. Turing, J. B. Kruskal, Jon M. Kleinberg, Eva
        Tardos e Daniel R. Figueiredo. Formato assumido do arquivo de
        nomes: a linha i (1-based) e o nome do vertice i; se o arquivo do
        site vier diferente, o ajuste fica na funcao encontrar_rotulos,
        no comeco do programa.

A API completa da biblioteca (buscas, componentes, diametro, estatisticas,
arquivos de saida) esta no READ_ME.txt da pasta "Trabalho 1 (Implementação
em C)"; as adaptacoes Python -> C no ADAPTACOES_C.txt (item 14 e a Parte 2)
e a fundamentacao teorica no THEORETICAL_APPROACH.md, la tambem.
