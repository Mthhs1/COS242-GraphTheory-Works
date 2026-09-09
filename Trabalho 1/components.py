"""
Requisito 6 do enunciado: componentes conexas.

Descobre o numero de componentes, o tamanho de cada uma e a lista de vertices
que a compoe, devolvidas em ordem DECRESCENTE de tamanho.

Custo: O(n + m). Cada vertice entra na fila uma unica vez (e marcado na
descoberta) e cada aresta e examinada duas vezes, uma na lista de cada
extremidade.
"""

from my_queue import Queue
from graph_utils import neighbors


def connected_components(graph, adj_list: bool = False):
    """
    Retorna a lista de componentes conexas, cada uma como lista de rotulos
    1-based ordenados, e o conjunto todo ordenado por tamanho decrescente.
    """
    n = len(graph)
    visited = [False] * n
    components = []

    for source in range(n):
        if visited[source]:
            continue

        # BFS a partir de cada vertice ainda nao visitado: cada varredura
        # esgota exatamente uma componente.
        component = []
        queue = Queue()
        queue.enqueue(source)
        visited[source] = True

        while not queue.isEmpty():
            current = queue.dequeue()
            component.append(current + 1)  # rotulo 1-based, como no arquivo

            for neighbor in neighbors(graph, current, adj_list):
                if not visited[neighbor]:
                    visited[neighbor] = True
                    queue.enqueue(neighbor)

        component.sort()
        components.append(component)

    components.sort(key=len, reverse=True)
    return components


def component_of(graph, adj_list: bool = False):
    """
    Vetor auxiliar: component_id[v] diz a qual componente (indice na lista
    devolvida por connected_components) o vertice 0-based v pertence.
    Util para o diametro, que so faz sentido dentro de uma componente.
    """
    components = connected_components(graph, adj_list)
    component_id = [-1] * len(graph)

    for index, component in enumerate(components):
        for label in component:
            component_id[label - 1] = index

    return components, component_id


def summary(graph, adj_list: bool = False):
    """Resumo textual das componentes, no formato usado no arquivo de saida."""
    components = connected_components(graph, adj_list)
    lines = [f"Numero de componentes conexas: {len(components)}"]

    if components:
        lines.append(f"Maior componente: {len(components[0])} vertices")
        lines.append(f"Menor componente: {len(components[-1])} vertices")

    lines.append("")
    for index, component in enumerate(components, start=1):
        lines.append(f"Componente {index} ({len(component)} vertices):")
        lines.append("  " + " ".join(str(v) for v in component))

    return "\n".join(lines)


if __name__ == "__main__":
    from pathlib import Path
    from graph import Graph

    example = Path(__file__).with_name("graph_exemplo.txt")

    for use_list in (False, True):
        rep = "lista de adjacencia" if use_list else "matriz de adjacencia"
        g = Graph(example, adj_list=use_list)
        print(f"--- {rep} ---")
        print(summary(g.graph, use_list))
        print()
