"""
Requisito 5 do enunciado: distancia entre dois vertices e diametro do grafo.

A distancia usa a BFS como primitiva, como pede o enunciado: em grafo nao
ponderado, o nivel da BFS E a distancia minima.

Diametro exato: BFS a partir de cada vertice, O(n * (n + m)). Inviavel nos
grafos grandes do estudo de caso, por isso ha tambem a versao aproximada.

Diametro aproximado (heuristica de dupla varredura): a partir de um vertice
qualquer, a BFS acha o vertice mais distante u; uma segunda BFS a partir de u
da uma estimativa por baixo do diametro. E barato (2 BFS por componente) e
costuma acertar ou chegar muito perto em grafos reais.
"""

from my_queue import Queue
from graph_utils import neighbors

INFINITO = -1  # marcador de "inalcancavel", coerente com levels = -1 do BFS.py


def bfs_distances(graph, source, adj_list: bool = False):
    """
    Distancias de `source` (0-based) para todos os vertices.
    Retorna lista onde nao alcancado = INFINITO.
    """
    n = len(graph)
    distances = [INFINITO] * n
    distances[source] = 0

    queue = Queue()
    queue.enqueue(source)

    while not queue.isEmpty():
        current = queue.dequeue()
        for neighbor in neighbors(graph, current, adj_list):
            if distances[neighbor] == INFINITO:
                distances[neighbor] = distances[current] + 1
                queue.enqueue(neighbor)

    return distances


def distance(graph, u, v, adj_list: bool = False, one_based: bool = True):
    """
    Distancia entre u e v. Por padrao recebe rotulos 1-based (como no arquivo
    de entrada) e devolve INFINITO se estiverem em componentes diferentes.
    """
    if one_based:
        u, v = u - 1, v - 1
    return bfs_distances(graph, u, adj_list)[v]


def _eccentricity(graph, source, adj_list):
    """Maior distancia finita a partir de source, e o vertice que a realiza."""
    distances = bfs_distances(graph, source, adj_list)
    best_vertex, best_distance = source, 0

    for vertex, d in enumerate(distances):
        if d != INFINITO and d > best_distance:
            best_vertex, best_distance = vertex, d

    return best_distance, best_vertex


def diameter(graph, adj_list: bool = False):
    """
    Diametro exato: maior distancia minima entre qualquer par alcancavel.
    Em grafo desconexo, e o maior diametro entre as componentes (distancias
    infinitas sao ignoradas).
    """
    largest = 0
    for source in range(len(graph)):
        eccentricity, _ = _eccentricity(graph, source, adj_list)
        if eccentricity > largest:
            largest = eccentricity
    return largest


def approximate_diameter(graph, adj_list: bool = False):
    """
    Estimativa por dupla varredura, aplicada uma vez por componente.
    O valor devolvido e sempre <= diametro real (e uma cota inferior).
    """
    from components import connected_components

    estimate = 0
    for component in connected_components(graph, adj_list):
        start = component[0] - 1  # rotulo 1-based -> indice

        _, farthest = _eccentricity(graph, start, adj_list)
        second_pass, _ = _eccentricity(graph, farthest, adj_list)

        if second_pass > estimate:
            estimate = second_pass

    return estimate


if __name__ == "__main__":
    from pathlib import Path
    from graph import Graph

    example = Path(__file__).with_name("graph_exemplo.txt")

    for use_list in (False, True):
        rep = "lista de adjacencia" if use_list else "matriz de adjacencia"
        g = Graph(example, adj_list=use_list)
        print(f"--- {rep} ---")
        print("d(1,2) =", distance(g.graph, 1, 2, use_list))
        print("d(1,3) =", distance(g.graph, 1, 3, use_list))
        print("d(3,4) =", distance(g.graph, 3, 4, use_list))
        print("diametro exato       =", diameter(g.graph, use_list))
        print("diametro aproximado  =", approximate_diameter(g.graph, use_list))
        print()
