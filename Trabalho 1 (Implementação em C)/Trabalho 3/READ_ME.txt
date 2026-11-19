================================================================================
READ_ME - Parte 3 em C: digrafos com pesos, Bellman-Ford e Floyd-Warshall
(Trabalho 3, COS 242)
================================================================================

Esta pasta e a entrega da Parte 3 (trabalho-P3.pdf): a biblioteca da Parte 2
estendida para GRAFOS DIRECIONADOS COM PESOS (o parametro `direcionado` na
leitura e o grafo transposto), com as buscas e o Dijkstra valendo para
digrafos, e com dois algoritmos novos para pesos negativos: BELLMAN-FORD
(distancias e caminho minimo DE TODOS os vertices para um vertice
especifico, como pede o enunciado, ou de uma fonte para todos) e
FLOYD-WARSHALL (todos os pares), os dois detectando CICLO NEGATIVO.

Os ESTUDOS DE CASO da Parte 3 (secao 3 do enunciado) ficam em uma pasta
EXTERNA a esta, como combinado com o professor: "../Trabalho 3 - Estudos
de caso" (p3_estudos.c, rodar_estudos_p3.sh, gerar_tabelas_p3.py; make
estudos roda as quatro questoes em todos os grafos de ../../Grafos/P3,
na raiz do repositorio). O Makefile de la compila contra os fontes desta
pasta, sem copia-los. O restante e self-contained: a
biboteca inteira (estendida), os testes e os demos.


--------------------------------------------------------------------------------
1. ESTRUTURA DAS PASTAS E COMPILACAO
--------------------------------------------------------------------------------
    DataStructures/   no da lista (com o peso), fila, pilha e heap de minimos
                      com decrease-key (tudo da Parte 2, sem mudancas)
    graph_utils.h     o iterador de vizinhanca (NeighborIter): a unica parte
                      do codigo que sabe a diferenca entre a lista e a matriz
                      e onde moram os pesos - entrega (vizinho, peso). Na
                      Parte 3, a vizinhanca que ele entrega e a de SAIDA: a
                      direcao foi decidida na leitura e BFS, DFS, Dijkstra,
                      Bellman-Ford e Floyd-Warshall respeitam-na sem uma
                      linha a mais neles
    Algorithms/       buscas, componentes, distancia/diametro e estatisticas
                      (Parte 1), dijkstra (Parte 2) e os NOVOS bellman_ford.c
                      e floyd_warshall.c (Parte 3)
    bin/              executaveis gerados pelo make (criada ao compilar)
    raiz              a "classe" Graph (agora com o campo `directed` e o
                      Graph_reverse), a leitura do arquivo (agora com o
                      parametro direcionado), utilitarios, test_parte3.c,
                      os grafos de exemplo (.txt) e este guia

Requisitos: gcc e make.

    make                    -> compila os testes da Parte 3 e os demos novos
    make test               -> compila e roda os testes da Parte 3
    make clean              -> remove a pasta bin/

Demos (constroem e rodam):
    make bellman_ford_demo     bellman-ford fonte x alvo, com ciclo negativo
    make floyd_warshall_demo   todos os pares, com caminho reconstruido

Os executaveis ficam em bin/ e rodam A PARTIR DESTA PASTA (caminhos
relativos; os grafos de exemplo ficam na raiz desta pasta).


--------------------------------------------------------------------------------
2. FORMATO DO ARQUIVO DE ENTRADA E O PARAMETRO DIRECIONADO
--------------------------------------------------------------------------------
A primeira linha e o numero de vertices; cada linha seguinte e uma aresta
"a b" ou "a b peso" (peso = numero em ponto flutuante, podendo ser
negativo). Sem a terceira coluna, a aresta vale 1. Lacos ("v v") sao
ignorados e arestas repetidas sao fundidas (vale a ultima linha), como nas
Partes 1 e 2.

O NOVO da Parte 3 e o parametro (passado pelo usuario, como pede o
enunciado) que indica se o grafo e ou nao direcionado:

    Graph *g = Graph_create(arquivo, adj_list, direcionado);

    direcionado == false  a aresta "a b" existe nas duas direcoes, como
                          nas Partes 1 e 2 (os arquivos antigos continuam
                          validos e com os mesmos resultados)
    direcionado == true   a aresta "a b" so existe de a para b: a direcao
                          segue a ordem dos identificadores em cada linha
                          (do primeiro para o segundo vertice). "b a", se
                          tambem estiver no arquivo, e uma aresta DIFERENTE,
                          nao uma duplicata

A mesma regra vale para o peso negativo: a leitura aceita (agora tambem o
Bellman-Ford e o Floyd-Warshall); so o Dijkstra e que recusa, como antes.


--------------------------------------------------------------------------------
3. O QUE A PARTE 3 ACRESCENTOU (E COMO USAR)
--------------------------------------------------------------------------------
Digrafos nas buscas e no Dijkstra: nenhuma linha mudou neles. A vizinhanca
de SAIDA ja carrega a direcao, e BFS, DFS e Dijkstra seguem os arcos. O
que mudou foi a leitura (graf_gen.c: no digrafo, cada linha "a b" escreve
UM no na lista de a / UMA celula [a][b] na matriz, em vez dos dois) e o
iterador (documentado: vizinhanca de saida).

Operacoes da Parte 1 que sao para grafos NAO direcionados (componentes
conexas, distancia em arestas, diametro, estatisticas, arquivo de saida)
recusam digrafos com mensagem de erro - num digrafo o iterador so ve
vizinhos de saida e elas devolveriam numeros sem sentido (o certo seria
componentes FORTMENTE conexas, que nao fazem parte do trabalho).

    Graph *Graph_reverse(Graph *g);
        O GRAFO TRANSPOSTO (todas as arestas invertidas), na mesma
        representacao. E o ingrediente do "de todos para um": a distancia
        de v ao alvo t no grafo e a distancia de t a v no transposto. Na
        lista, o mesmo bloco unico de nos das duas passadas; como a aresta
        v->u vira u->v com v percorrido em ordem crescente, cada trecho
        nasce ordenado sem qsort. Em grafo nao direcionado, o transposto e
        uma copia.

    BellmanFordResult *Graph_bellman_ford(Graph *g, int fonte);
    BellmanFordResult *Graph_bellman_ford_to(Graph *g, int alvo);
    BellmanFordResult *bellman_ford(Graph *g, int fonte);      (0-based)
    BellmanFordResult *bellman_ford_to(Graph *g, int alvo);    (0-based)
        Distancias minimas e arvore de caminhos minimos COM PESOS
        NEGATIVOS, e se existe ciclo negativo. O enunciado pede o problema
        "de todos os vertices para um especifico": bellman_ford_to resolve
        sobre o transposto (ou direto, se o grafo nao e direcionado);
        bellman_ford e o classico "de uma fonte para todos", que tambem
        serve para o "todos os pares" do estudo de caso (uma chamada por
        fonte).

        As DUAS otimizacoes discutidas em aula:
        1. MEMORIA: em vez da tabela M[k][v] (distancia com no maximo k
           arestas, para todos os k - O(n^2)), um so vetor M[v] com as
           estimativas correntes, atualizado no lugar - O(n), como o
           Dijkstra. Cada relaxamento ja le o valor mais novo.
        2. TEMPO - SO OS VERTICES ATIVOS: M[v] so pode melhorar se algum
           predecessor u melhorou desde a ultima vez que as arestas de u
           foram relaxadas. Cada passada relaxa so as arestas de saida dos
           vertices ATIVOS (os que mudaram), em ordem crescente e no lugar;
           pular os inativos nao muda nenhum valor, so o trabalho. Sem
           ativos, o algoritmo para (a TERMINACAO PRECOCE, sem precisar de
           uma passada extra so para confirmar que nada muda). (Se
           houvesse ciclo negativo alcancavel, sempre daria para melhorar
           girando o ciclo: parar cedo nunca esconde ciclo negativo.) O
           campo `passadas` do resultado mostra o ganho (<= n-1).

        Ciclo negativo: uma passada a mais, so para detectar. O BF flaga
        os ciclos que a FONTE alcanca (no _to, os que CHEGAM ao alvo) -
        ciclo fora do alcance nao participa de nenhum caminho do
        problema. Com negative_cycle == true, dist/parents nao sao
        confiaveis.

        A arvore: no bellman_ford, parents[v] e o vertice ANTES de v no
        caminho fonte->v (arvore enraizada na fonte, como a do Dijkstra);
        no bellman_ford_to, parents[v] e o vertice DEPOIS de v no caminho
        v->alvo (a arvore aponta para o alvo).

    int *bellman_ford_path(BellmanFordResult *r, int alvo, int *comprimento);
    int *bellman_ford_path_to(BellmanFordResult *r, int origem, int *comprimento);
        Caminho minimo fonte->alvo (pela arvore de pais, como o
        dijkstra_path) e origem->alvo (pela arvore que aponta para o
        alvo, lida para a frente). NULL e 0 se nao ha caminho.

    FloydWarshallResult *Graph_floyd_warshall(Graph *g);
    FloydWarshallResult *floyd_warshall(Graph *g);
        Distancia e arvore de caminhos minimos entre TODOS os pares, e se
        o grafo tem ciclo negativo. O(n^3) de tempo.

        A otimizacao de MEMORIA discutida em aula: a DP "obvia" guardaria
        D[k][i][j] para todos os k - um tensor n^3. Como a passada k so
        le a passada k-1 (e as linhas/colunas do intermediario k nao
        mudam na passada k), UMA matriz dist[i][j] atualizada in place,
        com o laco k mais externo, faz o mesmo servico com O(n^2) - e o
        pred[i][j], que reconstrui os caminhos, tambem e uma unica
        matriz n^2.

        Otimizacoes de tempo (mesmas distancias): a matriz e um bloco
        CONTIGUO (dist[i] continua sendo a linha i); o laco interno nao
        testa infinito (INFINITO_PESO e o +inf do IEEE e inf + x = inf),
        o que deixa o compilador vetoriza-lo; e o triplo laco roda EM
        BLOCOS de 64 x 64 (as tres fases: diagonal, linha/coluna, resto),
        que cabem no cache - o FW classico relê a matriz inteira a cada k.
        Medido em digrafo sintetico com n = 3000: 21,7 s -> 5,8 s. Em
        empate de custo, a ordem em blocos pode escolher OUTRO caminho
        minimo (outro pred, mesmo custo); com n <= 64 a ordem e a classica.

        A arvore A PARTIR DE CADA vertice: pred[i][j] = predecessor de j
        no caminho minimo i->j; a LINHA i de pred e o vetor de pais da
        arvore enraizada em i. Ciclo negativo: dist[i][i] < 0 para algum
        i - deteccao GLOBAL (qualquer ciclo do grafo aparece no vertice
        dele), ao contrario do BF, que so ve o que a fonte alcanca.

    int *floyd_warshall_path(FloydWarshallResult *r, int origem, int destino,
                             int *comprimento);
        Caminho minimo origem->destino pela matriz de predecessores.
        NULL e 0 se nao ha caminho (ou se a arvore esta corrompida por
        ciclo negativo: os helpers limitam a caminhada a n passos).

    Johnson *johnson_preparar(Graph *g);
    DijkstraResult *johnson_fonte(Johnson *j, Graph *g, int fonte);
        OTIMIZACAO (alem do pedido): todos os pares com pesos negativos
        pelo algoritmo de JOHNSON, O(nm log n) - contra O(n^3) do
        Floyd-Warshall e O(n^2 m) de n Bellman-Fords. johnson_preparar roda
        UM Bellman-Ford a partir de um vertice virtual ligado a todos por
        peso 0 (bellman_ford_potencial: todos comecam em 0 e ativos) e
        guarda o potencial h; ele tambem detecta ciclo negativo em
        QUALQUER parte do grafo. johnson_fonte roda o MESMO Dijkstra com
        heap da Parte 2 sobre os pesos reduzidos peso + h(u) - h(v) >= 0
        (dijkstra_reduzido: o laco le o potencial ao ler cada aresta; o
        grafo nao e copiado) e devolve d(s,v) = d'(s,v) - h(s) + h(v).

Testes: ./bin/test_parte3 - leitura de digrafos nas duas representacoes
(arco duplicado vale a ultima, "a b" e "b a" sao arcos diferentes, laco
ignorado, MESMO arquivo lido dos dois jeitos), BFS/DFS/Dijkstra em digrafo
(com alcance assimetrico), Bellman-Ford fonte e alvo com pesos negativos,
as passadas da terminacao precoce, ciclos negativos (alcancado da fonte,
que chega ao alvo, inalcancavel - e a diferenca para o veredito global do
Floyd-Warshall), Floyd-Warshall completo com arvores e caminhos, o
transposto (e o transposto do transposto), e a REGRESSAO: os grafos das
Partes 1 e 2, lidos como nao direcionados, continuam com os valores
conhecidos. Os testes ainda cruzam os algoritmos entre si (BF = Dijkstra
sem pesos negativos; BF = linha do FW; BF-to = coluna do FW; Johnson =
linha do FW e mesmo veredito de ciclo negativo) e as duas representacoes
entre si, em todos os grafos de exemplo. graph_teste_digrafo_150.txt (150
vertices, pesos negativos sem ciclo) passa do tamanho de um bloco do FW:
ali se confere FW em blocos = BF de cada fonte = Johnson, e que todo
caminho reconstruido pelo pred custa a distancia. Os 176 testes da
Parte 1 e os da Parte 2 continuam passando nas pastas deles.

Grafos de exemplo desta pasta:
    graph_exemplo_direcionado.txt         a Figura 1 da Parte 2 lida como
                                          DIGRAFO (tem o ciclo negativo
                                          3->4->5->3 = -2.2)
    graph_teste_direcionado.txt           digrafo positivo com alcance
                                          assimetrico (ninguem alcanca o 7)
    graph_teste_direcionado_negativo.txt  pesos negativos SEM ciclo
                                          negativo (d(1,5) = -1)
    graph_teste_direcionado_duplicatas.txt duplicata de arco, arco reverso
                                          distinto e laco; lido tambem
                                          como nao direcionado
    graph_teste_ciclo_negativo.txt        ciclo negativo alcancado da
                                          fonte 1 e do alvo 4
    graph_teste_ciclo_negativo_inalcancavel.txt ciclo negativo que a fonte
                                          1 nao alcanca e que nao chega ao
                                          alvo 1 (o FW flaga, o BF nao)
    graph_teste_rotulo_invalido.txt       aresta com vertice fora de [1,n]:
                                          a leitura recusa (antes corrompia
                                          a memoria)
    graph_exemplo.txt, graph_exemplo_pesos*.txt  os da Parte 2, para a
                                          regressao (nao direcionados)

A API completa da biblioteca (buscas, componentes, diametro, estatisticas,
Dijkstra) esta no READ_ME.txt da pasta "Trabalho 1 (Implementação em C)";
as adaptacoes Python -> C no ADAPTACOES_C.txt e a fundamentacao teorica no
THEORETICAL_APPROACH.md, la tambem.
