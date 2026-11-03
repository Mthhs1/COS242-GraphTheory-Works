# COS242 — Biblioteca de grafos (Trabalhos 1 e 2)

Trabalho da disciplina Teoria dos Grafos (COS242, UFRJ): uma biblioteca para
grafos não-direcionados com matriz e lista de adjacência, BFS, DFS, distâncias,
diâmetro, componentes conexas e estatísticas (Parte 1), e grafos com pesos nas
arestas com distância e caminho mínimo por Dijkstra, com vetor e com heap
(Parte 2), além dos programas dos estudos de caso das duas partes.

Dupla: Matheus da Silva e Luiz Felipe Cantanhede Cristino.

| Pasta | Conteúdo |
|---|---|
| [`Trabalho 1 (Implementação em C)/`](<Trabalho 1 (Implementação em C)/>) | a entrega da Parte 1: a biblioteca em C, os testes da Parte 1 e a automação dos estudos de caso dela (os fontes da biblioteca — estendidos com pesos e Dijkstra para a Parte 2 — são idênticos aos da pasta do Trabalho 2) |
| [`Trabalho 2/`](<Trabalho 2/>) | a entrega da Parte 2, de forma independente: os mesmos fontes da biblioteca, o `test_parte2`, os demos novos (heap e dijkstra) e os programas e a automação dos estudos de caso da Parte 2; o Makefile tem só os alvos dela |
| [`Trabalho 1/`](<Trabalho 1/>) | protótipo em Python, de onde a versão em C foi transcrita |
| `Grafos/` | grafos do estudo de caso (não versionados; baixe do site da disciplina) |

## Compilar e testar

Requisitos: `gcc` e `make` (Linux ou WSL).

```bash
cd "Trabalho 1 (Implementação em C)"
make test

cd "../Trabalho 2"
make test
```

## Estudos de caso

Coloque os grafos do site da disciplina na pasta `Grafos/`, na raiz do repositório
(os de duas colunas são os da Parte 1; os com pesos têm três colunas), e rode:

```bash
cd "Trabalho 1 (Implementação em C)"   # Parte 1: questões 1 a 7 em cada grafo
make estudos

cd ../Trabalho\ 2                       # Parte 2: só grafos com pesos
make estudos
```

Na Parte 2, o `rodar_estudos_p2.sh` roda as questões 1 e 2 da Parte 2 nos grafos
com pesos, e a questão da rede de colaboração quando `REDE_COLABORACAO` (o grafo)
e `REDE_NOMES` (o arquivo de nomes dos vértices) apontam para os arquivos. Cada
pasta gera a sua tabela por questão em `resultados/tabelas.md`
([Parte 1](<Trabalho 1 (Implementação em C)/resultados/tabelas.md>) e
[Parte 2](<Trabalho 2/resultados/tabelas.md>)); a saída de cada programa fica em
`resultados/brutos/` e a máquina usada em `resultados/ambiente.txt`.

## Organização da biblioteca

Em C não há classes. Cada "classe" é uma `struct` com funções de mesmo prefixo, que
recebem a struct como primeiro argumento (o `self` do protótipo em Python). Funções
`static` fazem o papel de métodos privados.

| Módulo | Papel |
|---|---|
| `graph.h` / `graph.c` | `Graph`: número de vértices, as flags da representação e dos pesos, uma `union` com a lista (`Node **`) ou a matriz (`MatrixCell **`, 1 byte por célula) e a matriz de pesos (`double **`, alocada sob demanda). É a API pública (`Graph_create`, `Graph_BFS`, `Graph_DFS`, `Graph_distance`, `Graph_diameter`, `Graph_connected_components`, `Graph_stats`, `Graph_write_output`, `Graph_write_search_tree`, `Graph_dijkstra`). |
| `graph_gen.c` | leitura do arquivo (bufferizada, com a terceira coluna opcional de pesos da Parte 2) e construção das duas representações (lista em duas passadas num único bloco contíguo de nós, sem overhead por nó do alocador) |
| `Algorithms/` | BFS e DFS (uma implementação cada, que serve às duas representações), componentes conexas, distância e diâmetro, estatísticas e arquivos de saída, e Dijkstra com pesos em duas formas (vetor e heap) sobre uma única implementação (`DistStore`) |
| `DataStructures/` | nó da lista (com peso da aresta), fila, pilha e heap de mínimos com decrease-key (`posicao[v]` em O(1)) |
| `graph_utils.h` | `NeighborIter`, o iterador de vizinhança que percorre a lista ou a linha da matriz sem alocar — a porta única que faz cada algoritmo ser escrito uma vez só — e que, na Parte 2, entrega também o peso da aresta |

Resultados das buscas e das análises voltam em structs próprias: `GraphSearch` (ordem, pais
e níveis), `Components`, `GraphStats` e `DijkstraResult` (distâncias e pais). Os rótulos de vértice na API são 1-based, como no
arquivo de entrada. A API completa, o formato do arquivo e exemplos de uso estão em
[`Trabalho 1/READ_ME.txt`](<Trabalho 1 (Implementação em C)/READ_ME.txt>) e
[`Trabalho 2/READ_ME.txt`](<Trabalho 2/READ_ME.txt>). As diferenças em relação ao
protótipo em Python estão em
[`ADAPTACOES_C.txt`](<Trabalho 1 (Implementação em C)/ADAPTACOES_C.txt>).
