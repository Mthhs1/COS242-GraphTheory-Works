# COS242 — Biblioteca de grafos (Trabalho 1)

Trabalho da disciplina Teoria dos Grafos (COS242, UFRJ): uma biblioteca para
grafos não-direcionados com matriz e lista de adjacência, BFS, DFS, distâncias,
diâmetro, componentes conexas e estatísticas, e os programas dos estudos de caso.

Dupla: Matheus da Silva e Luiz Felipe Cantanhede Cristino.

| Pasta | Conteúdo |
|---|---|
| [`Trabalho 1 (Implementação em C)/`](<Trabalho 1 (Implementação em C)/>) | a biblioteca entregue, em C, os testes e os estudos de caso |
| [`Trabalho 1/`](<Trabalho 1/>) | protótipo em Python, de onde a versão em C foi transcrita |
| `Grafos/` | grafos do estudo de caso (não versionados; baixe do site da disciplina) |

## Compilar e testar

Requisitos: `gcc` e `make` (Linux ou WSL).

```bash
cd "Trabalho 1 (Implementação em C)"
make test
```

## Estudos de caso

Coloque `grafo_1.txt` a `grafo_6.txt` na pasta `Grafos/`, na raiz do repositório, e rode:

```bash
cd "Trabalho 1 (Implementação em C)"
make estudos
```

O script roda as questões 1 a 7 do enunciado em cada grafo, nas duas representações,
e gera uma tabela por questão em
[`resultados/tabelas.md`](<Trabalho 1 (Implementação em C)/resultados/tabelas.md>).
A saída de cada programa fica em `resultados/brutos/`, e a máquina usada em
`resultados/ambiente.txt`.

## Organização da biblioteca

Em C não há classes. Cada "classe" é uma `struct` com funções de mesmo prefixo, que
recebem a struct como primeiro argumento (o `self` do protótipo em Python). Funções
`static` fazem o papel de métodos privados.

| Módulo | Papel |
|---|---|
| `graph.h` / `graph.c` | `Graph`: número de vértices, a flag da representação e uma `union` com a lista (`Node **`) ou a matriz (`int **`). É a API pública (`Graph_create`, `Graph_BFS`, `Graph_DFS`, `Graph_distance`, `Graph_diameter`, `Graph_connected_components`, `Graph_stats`, `Graph_write_output`, `Graph_write_search_tree`). |
| `graph_gen.c` | leitura do arquivo (bufferizada) e construção das duas representações |
| `Algorithms/` | BFS e DFS (uma versão por representação), componentes conexas, distância e diâmetro, estatísticas e arquivos de saída |
| `DataStructures/` | nó da lista, fila e pilha |
| `graph_utils.c` | `neighbors()` e `degree()`, a porta única para a vizinhança nas duas representações, e utilitários de memória e texto |

Resultados das buscas e das análises voltam em structs próprias: `GraphSearch` (ordem, pais
e níveis), `Components` e `GraphStats`. Os rótulos de vértice na API são 1-based, como no
arquivo de entrada. A API completa, o formato do arquivo e um exemplo de uso estão em
[`READ_ME.txt`](<Trabalho 1 (Implementação em C)/READ_ME.txt>). As diferenças em relação ao
protótipo em Python estão em
[`ADAPTACOES_C.txt`](<Trabalho 1 (Implementação em C)/ADAPTACOES_C.txt>).
