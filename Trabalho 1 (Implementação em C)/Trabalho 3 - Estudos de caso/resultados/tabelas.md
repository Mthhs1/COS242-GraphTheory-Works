# Estudos de caso da Parte 3 — resultados

Ambiente de execução:

```
data: 2026-11-21 09:18
commit: tp3-estudos
sistema: Linux 6.18.58.1-microsoft-standard-WSL2 x86_64
processador: 12th Gen Intel(R) Core(TM) i5-12400F
memoria total: 7.7 GB
compilador: gcc (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0
flags: -std=c11 -Wall -Wextra -O3 -march=native
rodadas: 10
```

## Grafos

| Grafo | Vértices | Arestas | Pesos negativos |
|---|--:|--:|--:|
| grafo_W_1 | 25.000 | 549.953 | sim |
| grafo_W_2 | 25.000 | 824.964 | nao |
| grafo_W_3 | 100.000 | 6.099.941 | nao |
| grafo_W_4 | 1.000.000 | 30.999.978 | nao |
| grafo_W_5 | 10.000.000 | 39.999.995 | nao |

## Questão 1 — Bellman-Ford: distância até o vértice 100

| Grafo | d(10, 100) | d(20, 100) | d(30, 100) |
|---|--:|--:|--:|
| grafo_W_1 | indefinida (ciclo negativo) | indefinida (ciclo negativo) | indefinida (ciclo negativo) |
| grafo_W_2 | 10,470 (11 arestas) | 8,480 (6 arestas) | 8,710 (11 arestas) |
| grafo_W_3 | 4,630 (8 arestas) | 5,420 (8 arestas) | 4,670 (7 arestas) |
| grafo_W_4 | 9,040 (13 arestas) | 8,830 (11 arestas) | 9,230 (14 arestas) |
| grafo_W_5 | 37,850 (22 arestas) | 42,630 (22 arestas) | 37,860 (30 arestas) |

Entre parênteses, o número de arestas do caminho mínimo. Com ciclo negativo que chega ao 100, as distâncias não são definidas.

## Questão 2 — grafo_W_1

O grafo tem ciclo negativo (dist[i][i] < 0 para algum i): as distâncias não são definidas.

## Questão 2 — Floyd-Warshall, grafo_W_2

| de \ para | 10 | 20 | 30 | 40 | 50 |
|---|--:|--:|--:|--:|--:|
| 10 | 0,000 | 11,730 | 8,140 | 9,490 | 7,080 |
| 20 | 3,840 | 0,000 | 7,030 | 8,300 | 4,700 |
| 30 | 5,790 | 12,550 | 0,000 | 8,300 | 6,330 |
| 40 | 5,050 | 12,320 | 5,360 | 0,000 | 5,710 |
| 50 | 4,480 | 12,230 | 7,790 | 8,260 | 0,000 |

## Questão 2 — Floyd-Warshall, grafo_W_3

| de \ para | 10 | 20 | 30 | 40 | 50 |
|---|--:|--:|--:|--:|--:|
| 10 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 20 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 30 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 40 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 50 | ∞ | ∞ | ∞ | ∞ | ∞ |

## Questão 2 — Floyd-Warshall, grafo_W_4

| de \ para | 10 | 20 | 30 | 40 | 50 |
|---|--:|--:|--:|--:|--:|
| 10 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 20 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 30 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 40 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 50 | ∞ | ∞ | ∞ | ∞ | ∞ |

## Questão 2 — Floyd-Warshall, grafo_W_5

| de \ para | 10 | 20 | 30 | 40 | 50 |
|---|--:|--:|--:|--:|--:|
| 10 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 20 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 30 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 40 | ∞ | ∞ | ∞ | ∞ | ∞ |
| 50 | ∞ | ∞ | ∞ | ∞ | ∞ |

## Questão 3 — tempo médio (10 rodadas)

| Grafo | Bellman-Ford (todos → 100) | Floyd-Warshall |
|---|--:|--:|
| grafo_W_1 | 0,011 s | 2,254 s |
| grafo_W_2 | 0,030 s | 1.612,750 s |
| grafo_W_3 | 0,335 s | — |
| grafo_W_4 | 3,471 s | — |
| grafo_W_5 | 17,065 s | — |

O cronômetro cobre só o algoritmo (sem leitura nem escrita em disco). O Bellman-Ford "de todos para um" inclui a construção do grafo transposto.

## Questão 4 — todos os pares

| Grafo | n × Bellman-Ford | Floyd-Warshall | Johnson | Conferência |
|---|--:|--:|--:|--:|
| grafo_W_1 | 27,228 s | 2,254 s | 2,371 ms | ciclo negativo no grafo (johnson) - distancias nao definidas |
| grafo_W_2 | 319,993 s | 1.612,750 s | 333,308 s | 25000 fontes, 625000000 pares alcancaveis, maior diferenca 0, alcancabilidade divergente em 0 pares (BF x Johnson; FW inviavel) |
| grafo_W_3 | 9.324,098 s (est.) | — | 9.451,318 s (est.) | 20 fontes, 2000000 pares alcancaveis, maior diferenca 0, alcancabilidade divergente em 0 pares (BF x Johnson; FW inviavel) |
| grafo_W_4 | 1.137.210,501 s (est.) | — | 1.765.755,932 s (est.) | 5 fontes, 5000000 pares alcancaveis, maior diferenca 0, alcancabilidade divergente em 0 pares (BF x Johnson; FW inviavel) |
| grafo_W_5 | 160.716.247,753 s (est.) | — | 153.012.129,704 s (est.) | 3 fontes, 30000000 pares alcancaveis, maior diferenca 0, alcancabilidade divergente em 0 pares (BF x Johnson; FW inviavel) |

n × Bellman-Ford: uma execução por fonte. Johnson: um Bellman-Ford para o potencial e um Dijkstra com heap por fonte (otimização, além do pedido).
