"""
Utilitarios compartilhados pelos modulos de componentes, distancia e estatisticas.

A biblioteca suporta as duas representacoes exigidas pelo enunciado (matriz e
lista de adjacencia). Para nao duplicar cada algoritmo, tudo aqui recebe a flag
`adj_list` e usa `neighbors()` como unica porta de acesso a vizinhanca.
"""


def neighbors(graph, v, adj_list: bool = False):
    """Itera sobre os vizinhos de v (indices 0-based), nas duas representacoes."""
    if adj_list:
        node = graph[v].next
        while node is not None:
            yield node.value
            node = node.next
    else:
        row = graph[v]
        for u in range(len(row)):
            if row[u] == 1:
                yield u


def degree(graph, v, adj_list: bool = False):
    """Grau do vertice v."""
    if adj_list:
        d = 0
        node = graph[v].next
        while node is not None:
            d += 1
            node = node.next
        return d
    return sum(graph[v])


def all_degrees(graph, adj_list: bool = False):
    """Lista com o grau de cada vertice, na ordem dos indices."""
    return [degree(graph, v, adj_list) for v in range(len(graph))]


def edge_count(graph, adj_list: bool = False):
    """Numero de arestas. Pelo lema do aperto de mao: m = (1/2) * soma dos graus."""
    return sum(all_degrees(graph, adj_list)) // 2
