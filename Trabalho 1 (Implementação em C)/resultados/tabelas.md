# Estudos de caso — resultados

Ambiente de execução:

```
data: 2026-09-17 17:55
commit: f472062
sistema: Linux 6.18.33.2-microsoft-standard-WSL2 x86_64
processador: 12th Gen Intel(R) Core(TM) i5-12400F
nucleos: 12
memoria total: 7.7 GB
compilador: gcc (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0
flags: -std=c11 -Wall -Wextra -O2 -I. -IDataStructures -IAlgorithms
buscas por tipo (Q2/Q3): 100
```

## Grafos (requisito 2)

| Grafo | Vértices | Arestas | Grau mín. | Grau máx. | Grau médio | Mediana |
|---|--:|--:|--:|--:|--:|--:|
| grafo_1 | 10.000 | 109.921 | 8 | 43 | 21,98 | 22,0 |
| grafo_2 | 49.948 | 1.298.710 | 1 | 112 | 52,00 | 55,0 |
| grafo_3 | 375.000 | 765.615 | 1 | 15 | 4,08 | 4,0 |
| grafo_4 | 375.000 | 8.186.986 | 9 | 89 | 43,66 | 47,0 |
| grafo_5 | 4.843.750 | 13.168.911 | 1 | 22 | 5,44 | 5,0 |
| grafo_6 | 4.843.750 | 46.469.479 | 2 | 56 | 19,19 | 20,0 |

Laços e arestas repetidas dos arquivos são ignorados (grafo simples).

## Questão 1 — memória (MB)

| Grafo | Lista | Matriz (residente) | Matriz (alocada) |
|---|--:|--:|--:|
| grafo_1 | 7,7 | 95,8 | 95,4 |
| grafo_2 | 82,2 | 2.364,1 | 2.379,6 |
| grafo_3 | 62,9 | inviável (131,0 GB) | inviável (131,0 GB) |
| grafo_4 | 519,8 | inviável (131,0 GB) | inviável (131,0 GB) |
| grafo_5 | 1.022,9 | inviável (21.850,6 GB) | inviável (21.850,6 GB) |
| grafo_6 | 3.087,2 | inviável (21.850,6 GB) | inviável (21.850,6 GB) |

Memória residente do processo (VmRSS) depois de carregar menos a de antes. Na matriz, páginas que nunca recebem uma aresta não chegam a ocupar memória física, por isso a coluna "alocada" mostra o tamanho reservado (n² células de 1 byte). "Inviável" = a matriz não cabe na memória disponível da máquina.

## Questões 2 e 3 — tempo médio de uma busca (ms)

| Grafo | BFS lista | BFS matriz | DFS lista | DFS matriz |
|---|--:|--:|--:|--:|
| grafo_1 | 2,47 | 42,81 | 5,47 | 46,05 |
| grafo_2 | 14,64 | 346,16 | 33,05 | 368,46 |
| grafo_3 | 54,10 | inviável (131,0 GB) | 100,13 | inviável (131,0 GB) |
| grafo_4 | 208,33 | inviável (131,0 GB) | 388,98 | inviável (131,0 GB) |
| grafo_5 | 867,99 | inviável (21.850,6 GB) | 1.856,24 | inviável (21.850,6 GB) |
| grafo_6 | 2.250,32 | inviável (21.850,6 GB) | 4.167,78 | inviável (21.850,6 GB) |

Média de 100 buscas de cada tipo, partindo de vértices distintos espalhados por [1, n]. O cronômetro cobre apenas o algoritmo (sem leitura nem escrita).

## Questão 4 — pai dos vértices 10 / 20 / 30

| Grafo | BFS raiz 1 | BFS raiz 2 | BFS raiz 3 | DFS raiz 1 | DFS raiz 2 | DFS raiz 3 |
|---|--:|--:|--:|--:|--:|--:|
| grafo_1 | 2042 / 8382 / 2394 | 8935 / 9071 / 3555 | 7685 / 9543 / 5783 | 709 / 666 / 86 | 709 / 666 / 86 | 709 / 666 / 86 |
| grafo_2 | - / - / - | 1351 / - / - | - / 46738 / 12999 | - / - / - | 3946 / - / - | - / 217 / 3513 |
| grafo_3 | - / - / 141597 | 158403 / 75471 / - | 158403 / 319691 / - | - / - / 141597 | 192218 / 141526 / - | 106718 / 141526 / - |
| grafo_4 | 243865 / 370783 / 136244 | - / - / - | - / - / - | 12269 / 10738 / 1531 | - / - / - | - / - / - |
| grafo_5 | 1888350 / - / 2502539 | - / - / - | 1888350 / - / 191713 | 1888350 / - / 191713 | - / - / - | 1888350 / - / 2502539 |
| grafo_6 | - / - / - | 1677854 / 3607226 / 3898629 | - / - / - | - / - / - | 381031 / 431008 / 446011 | - / - / - |

"-" = vértice fora da componente da raiz. Vizinhos visitados em ordem crescente de rótulo.

## Questão 5 — distâncias

| Grafo | d(10, 20) | d(10, 30) | d(20, 30) |
|---|--:|--:|--:|
| grafo_1 | 3 | 3 | 4 |
| grafo_2 | sem caminho | sem caminho | 3 |
| grafo_3 | 9 | sem caminho | sem caminho |
| grafo_4 | 4 | 3 | 4 |
| grafo_5 | sem caminho | 9 | sem caminho |
| grafo_6 | 5 | 5 | 5 |

## Questão 6 — componentes conexas

| Grafo | Componentes | Maior (vértices) | Menor (vértices) |
|---|--:|--:|--:|
| grafo_1 | 1 | 10.000 | 10.000 |
| grafo_2 | 10 | 25.000 | 48 |
| grafo_3 | 2 | 250.000 | 125.000 |
| grafo_4 | 2 | 250.000 | 125.000 |
| grafo_5 | 5 | 2.500.000 | 156.250 |
| grafo_6 | 5 | 2.500.000 | 156.250 |

## Questão 7 — diâmetro

| Grafo | Aproximado | Tempo aprox. (s) | Exato | Tempo exato (s) |
|---|--:|--:|--:|--:|
| grafo_1 | 4 | 0,01 | 5 | 23,2 |
| grafo_2 | 20 | 0,13 | 20 | 787,2 |
| grafo_3 | 22 | 0,53 | não calculado | — |
| grafo_4 | 5 | 1,33 | não calculado | — |
| grafo_5 | 58 | 10,54 | não calculado | — |
| grafo_6 | 19 | 19,15 | não calculado | — |

Aproximado: dupla varredura (2 BFS por componente), uma cota inferior do diâmetro. Exato: BFS a partir de cada vértice, calculado só nos grafos menores. Medido na lista.

## Conferência entre representações

Questões 4 a 7 com respostas idênticas na lista e na matriz: grafo_1, grafo_2.
