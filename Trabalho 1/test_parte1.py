"""
Testes de verificacao da Parte 1.

Rodar com:  python test_parte1.py

Os valores esperados foram conferidos a mao a partir dos dois grafos de teste:

  graph_exemplo.txt          -> o grafo da Figura 1 do enunciado (n=5, m=5)
  graph_teste_desconexo.txt  -> n=7, m=5, tres componentes, um vertice isolado

O segundo grafo existe justamente porque m != n: o leitor original lia n linhas
de aresta, e o exemplo do enunciado (onde m == n) mascarava esse erro.
"""

from pathlib import Path

from graph import Graph
from graph_utils import all_degrees, edge_count
from graph_stats import graph_stats, median
from components import connected_components
from distance import distance, diameter, approximate_diameter, INFINITO

EXEMPLO = Path(__file__).with_name("graph_exemplo.txt")
DESCONEXO = Path(__file__).with_name("graph_teste_desconexo.txt")

falhas = []


def checar(descricao, obtido, esperado):
    ok = obtido == esperado
    marcador = "ok  " if ok else "FALHA"
    print(f"  [{marcador}] {descricao}: {obtido!r}")
    if not ok:
        falhas.append(f"{descricao}: obtido {obtido!r}, esperado {esperado!r}")


def testar_exemplo(adj_list):
    g = Graph(EXEMPLO, adj_list=adj_list).graph

    checar("graus", all_degrees(g, adj_list), [2, 2, 1, 1, 4])
    checar("numero de arestas", edge_count(g, adj_list), 5)

    s = graph_stats(g, adj_list)
    checar("grau minimo", s["grau_minimo"], 1)
    checar("grau maximo", s["grau_maximo"], 4)
    checar("grau medio", round(s["grau_medio"], 4), 2.0)
    checar("mediana de grau", s["mediana_grau"], 2.0)

    checar("componentes", connected_components(g, adj_list), [[1, 2, 3, 4, 5]])

    checar("d(1,2)", distance(g, 1, 2, adj_list), 1)
    checar("d(1,3)", distance(g, 1, 3, adj_list), 2)
    checar("d(3,4)", distance(g, 3, 4, adj_list), 2)
    checar("diametro", diameter(g, adj_list), 2)
    checar("diametro aproximado", approximate_diameter(g, adj_list), 2)


def testar_desconexo(adj_list):
    g = Graph(DESCONEXO, adj_list=adj_list).graph

    checar("graus", all_degrees(g, adj_list), [2, 2, 2, 0, 1, 2, 1])
    checar("numero de arestas", edge_count(g, adj_list), 5)

    s = graph_stats(g, adj_list)
    checar("grau minimo", s["grau_minimo"], 0)
    checar("grau maximo", s["grau_maximo"], 2)
    checar("grau medio", round(s["grau_medio"], 4), round(10 / 7, 4))
    checar("mediana de grau", s["mediana_grau"], 2.0)

    componentes = connected_components(g, adj_list)
    checar("numero de componentes", len(componentes), 3)
    checar("tamanhos (ordem decrescente)", [len(c) for c in componentes], [3, 3, 1])
    checar("menor componente", componentes[-1], [4])

    checar("d(1,3) mesma componente", distance(g, 1, 3, adj_list), 1)
    checar("d(5,7) mesma componente", distance(g, 5, 7, adj_list), 2)
    checar("d(1,5) componentes distintas", distance(g, 1, 5, adj_list), INFINITO)
    checar("diametro", diameter(g, adj_list), 2)


def testar_mediana():
    checar("mediana impar", median([1, 3, 2]), 2.0)
    checar("mediana par", median([1, 2, 3, 4]), 2.5)
    checar("mediana vazia", median([]), 0.0)


def testar_arvore_bfs():
    """A BFS a partir do vertice 5 (indice 4) alcanca todos em um nivel."""
    g = Graph(EXEMPLO, adj_list=True)
    _, parents, levels = g.BFS(4, generate_tree=True)

    checar("nivel da raiz", levels[4], 0)
    checar("niveis dos vizinhos de 5", [levels[i] for i in (0, 1, 2, 3)], [1, 1, 1, 1])
    checar("pais dos vizinhos de 5", [parents[i] for i in (0, 1, 2, 3)], [4, 4, 4, 4])


if __name__ == "__main__":
    for representacao, adj_list in (("matriz de adjacencia", False),
                                    ("lista de adjacencia", True)):
        print(f"\n=== graph_exemplo.txt / {representacao} ===")
        testar_exemplo(adj_list)

        print(f"\n=== graph_teste_desconexo.txt / {representacao} ===")
        testar_desconexo(adj_list)

    print("\n=== mediana ===")
    testar_mediana()

    print("\n=== arvore da BFS ===")
    testar_arvore_bfs()

    print("\n" + "=" * 50)
    if falhas:
        print(f"{len(falhas)} FALHA(S):")
        for f in falhas:
            print("  -", f)
        raise SystemExit(1)
    print("Todos os testes passaram.")
