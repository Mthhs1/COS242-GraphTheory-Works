# Otimizações além das que estão na biblioteca

Bancadas para medir alternativas que **não** estão em `../Trabalho 3`. Elas
são candidatas para a apresentação: cada uma tem o ganho medido e o motivo de
ter ficado fora da biblioteca, ou de entrar nela.

Máquina das medidas: i5-12400F (6 núcleos, 12 threads), 7,7 GB, WSL2, gcc 11.4
`-O3 -march=native -fopenmp`. Grafos sintéticos: `gerar_digrafos_teste.py`
(aleatório, n = 3000, m = 15000) e `gerar_grade.py` (grade 70 × 70, n = 4900,
caminhos longos). Todos têm pesos negativos e nenhum ciclo negativo.

```bash
make otimizacoes          # compila as duas bancadas em bin/
./bin/bancada_bf_todos_pares <grafo.txt> [amostra_de_fontes]
./bin/bancada_floyd_warshall <n>    # FW num digrafo aleatório de n vértices
```

## Floyd-Warshall, n = 3000

| Versão | Tempo | Ganho |
|---|--:|--:|
| original (linhas separadas, testa infinito no laço interno) | 22,0 s | 1× |
| matriz contígua, sem teste de infinito no laço interno | 10,3 s | 2,1× |
| **em blocos 64 × 64** (está na biblioteca) | 5,0 s | 4,4× |
| em blocos + 12 threads (OpenMP nas fases 2 e 3) | 1,5 s | 14,8× |

As distâncias são as mesmas em todas as versões. Os predecessores mudam só
quando há empate de custo, e todos os caminhos reconstruídos foram conferidos.
Os blocos de uma fase são independentes entre si: os da fase 2 dependem só do
bloco da diagonal, e os da fase 3 só dos da fase 2. Por isso cada fase
paraleliza sem trava.

## Bellman-Ford (uma fonte)

| Variante | Aleatório (n = 3000) | Grade (n = 4900) |
|---|--:|--:|
| **ativos em ordem crescente** (está na biblioteca) | 1,04 s* | 2,09 s* |
| varredura alternada (crescente, decrescente, …) | 1,12 s* | 4,25 s* |
| SPFA: fila FIFO de ativos | **0,61 s*** | **1,24 s*** |

\* Tempo para todas as n fontes, extrapolado de 600 fontes.

- **Varredura alternada:** é a ideia de Yen. Ela reduz as passadas (70 → 42 na
  grade), mas faz 3× mais relaxamentos. Ficou de fora.
- **SPFA:** é a mesma otimização dos vértices ativos, só que guardados numa
  fila, o que elimina a varredura de O(n) por passada. Também fez menos
  relaxamentos no grafo aleatório (23 mil contra 41 mil por fonte). O pior caso
  continua O(nm). A detecção de ciclo negativo mudaria: um vértice entrar na
  fila n vezes. Também mudaria o significado de "passadas". É a próxima
  candidata para a biblioteca.

## Todos os pares em paralelo

As n fontes são independentes. Um `#pragma omp parallel for` sobre as fontes
basta, porque cada execução tem os próprios vetores e o grafo só é lido.

| Grafo | n × BF sequencial | n × BF, 12 threads | Johnson sequencial | Johnson, 12 threads |
|---|--:|--:|--:|--:|
| aleatório, n = 3000 | 1,05 s | 0,32 s (3,3×) | 1,35 s | 0,34 s (4,0×) |
| grade, n = 4900 | 1,89 s | 0,53 s (3,5×) | 2,55 s | 0,49 s (5,2×) |

O ganho não chega a 12× porque cada fonte faz pouco trabalho: o tempo vai
para a memória, compartilhada entre os núcleos.

## Por que o paralelismo ficou fora da biblioteca

A questão 3 do enunciado compara o tempo dos algoritmos. Paralelizar só um
deles deixaria a comparação injusta, e paralelizar todos esconderia a
diferença de complexidade (n³ contra nm), que é o ponto da questão 4. Por isso
as medidas oficiais dos estudos de caso são sequenciais. O paralelismo entra na
apresentação como "o que daria para ganhar a mais".
