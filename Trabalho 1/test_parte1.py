"""
Testes de verificacao da Parte 1.

Rodar com:  python test_parte1.py

Os valores esperados foram conferidos a mao a partir dos dois grafos de teste:

  graph_exemplo.txt          -> o grafo da Figura 1 do enunciado (n=5, m=5)
  graph_teste_desconexo.txt  -> n=7, m=5, tres componentes, um vertice isolado

O segundo grafo existe justamente porque m != n: o leitor original lia n linhas
de aresta, e o exemplo do enunciado (onde m == n) mascarava esse erro.

A API publica da classe Graph usa rotulos 1-based (como no arquivo de entrada);
os vetores devolvidos (ordem, pais, niveis) sao indexados em 0.
"""

import tempfile
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


def checar_erro(descricao, funcao):
    try:
        funcao()
        checar(descricao, "sem excecao", "ValueError")
    except ValueError:
        checar(descricao, "ValueError", "ValueError")


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
    """A BFS a partir do vertice 5 (rotulo 1-based, indice 4) alcanca todos em um nivel."""
    g = Graph(EXEMPLO, adj_list=True)
    _, parents, levels = g.BFS(5, generate_tree=True)

    checar("nivel da raiz", levels[4], 0)
    checar("niveis dos vizinhos de 5", [levels[i] for i in (0, 1, 2, 3)], [1, 1, 1, 1])
    checar("pais dos vizinhos de 5", [parents[i] for i in (0, 1, 2, 3)], [4, 4, 4, 4])


def testar_contrato_de_retorno(adj_list):
    """Sem generate_tree a Graph devolve so a ordem; com ele, a tripla completa."""
    rep = "lista" if adj_list else "matriz"
    g = Graph(EXEMPLO, adj_list=adj_list)

    checar(f"BFS devolve lista [{rep}]", type(g.BFS(1)).__name__, "list")
    checar(f"DFS devolve lista [{rep}]", type(g.DFS(1)).__name__, "list")

    saida_bfs = g.BFS(1, generate_tree=True)
    saida_dfs = g.DFS(1, generate_tree=True)
    checar(f"BFS com arvore devolve tripla [{rep}]",
           (type(saida_bfs).__name__, len(saida_bfs)), ("tuple", 3))
    checar(f"DFS com arvore devolve tripla [{rep}]",
           (type(saida_dfs).__name__, len(saida_dfs)), ("tuple", 3))


def testar_target(adj_list):
    """BFS com target encerra na primeira visita ao alvo, nas duas representacoes."""
    rep = "lista" if adj_list else "matriz"
    g = Graph(EXEMPLO, adj_list=adj_list)

    ordem, pais, niveis = g.BFS(1, generate_tree=True, target=3)
    checar(f"ultimo visitado e o target [{rep}]", ordem[-1], 2)
    checar(f"nivel do target = d(1,3) [{rep}]", niveis[2], 2)
    checar(f"pai do target definido [{rep}]", pais[2] is not None, True)

    so_ordem = g.BFS(1, target=3)
    checar(f"target sem arvore devolve lista [{rep}]", type(so_ordem).__name__, "list")
    checar(f"lista termina no target [{rep}]", so_ordem[-1], 2)

    d = Graph(DESCONEXO, adj_list=adj_list)
    ordem_d = d.BFS(1, target=7)
    checar(f"alvo inalcancavel: componente completa [{rep}]", sorted(ordem_d), [0, 1, 2])
    checar(f"alvo inalcancavel: target fora da ordem [{rep}]", 6 in ordem_d, False)


def testar_rotulos_invalidos():
    g = Graph(EXEMPLO)
    for rotulo in (0, 6, -1):
        checar_erro(f"BFS rejeita rotulo {rotulo}", lambda r=rotulo: g.BFS(r))
    checar_erro("DFS rejeita rotulo 6", lambda: g.DFS(6))
    checar_erro("distance rejeita vertice 6", lambda: g.distance(1, 6))
    checar_erro("BFS rejeita target 6", lambda: g.BFS(1, target=6))


def testar_equivalencia_entre_representacoes():
    """DFS gera a mesma arvore nas duas representacoes; na BFS, os niveis coincidem."""
    gm = Graph(EXEMPLO, adj_list=False)
    gl = Graph(EXEMPLO, adj_list=True)

    for raiz in (1, 2, 3, 4, 5):
        ordem_m, pais_m, _ = gm.DFS(raiz, generate_tree=True)
        ordem_l, pais_l, _ = gl.DFS(raiz, generate_tree=True)
        checar(f"DFS ordem igual (raiz {raiz})", ordem_l, ordem_m)
        checar(f"DFS pais iguais (raiz {raiz})", pais_l, pais_m)

        _, _, niveis_m = gm.BFS(raiz, generate_tree=True)
        _, _, niveis_l = gl.BFS(raiz, generate_tree=True)
        checar(f"BFS niveis iguais (raiz {raiz})", niveis_l, niveis_m)


def testar_write_search_tree():
    """Requisito 4: arquivo com raiz, pai e nivel de cada vertice (rotulos 1-based)."""
    g = Graph(EXEMPLO, adj_list=True)

    with tempfile.TemporaryDirectory() as tmp:
        caminho = Path(tmp) / "arvore.txt"
        g.write_search_tree(caminho, 5)
        linhas = caminho.read_text().splitlines()

    checar("raiz registrada", linhas[0], "Raiz da busca: 5")
    checar("linha do vertice 1", linhas[3], "1 5 1")
    checar("linha da raiz", linhas[7], "5 - 0")
    checar("total de vertices na arvore", len(linhas) - 3, 5)


if __name__ == "__main__":
    for representacao, adj_list in (("matriz de adjacencia", False),
                                    ("lista de adjacencia", True)):
        print(f"\n=== graph_exemplo.txt / {representacao} ===")
        testar_exemplo(adj_list)

        print(f"\n=== graph_teste_desconexo.txt / {representacao} ===")
        testar_desconexo(adj_list)

        print(f"\n=== contrato de retorno / {representacao} ===")
        testar_contrato_de_retorno(adj_list)

        print(f"\n=== BFS com target / {representacao} ===")
        testar_target(adj_list)

    print("\n=== mediana ===")
    testar_mediana()

    print("\n=== arvore da BFS ===")
    testar_arvore_bfs()

    print("\n=== rotulos invalidos ===")
    testar_rotulos_invalidos()

    print("\n=== equivalencia entre representacoes ===")
    testar_equivalencia_entre_representacoes()

    print("\n=== arquivo da arvore de busca ===")
    testar_write_search_tree()

    print("\n" + "=" * 50)
    if falhas:
        print(f"{len(falhas)} FALHA(S):")
        for f in falhas:
            print("  -", f)
        raise SystemExit(1)
    print("Todos os testes passaram.")
