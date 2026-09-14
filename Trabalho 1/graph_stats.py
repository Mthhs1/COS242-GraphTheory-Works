"""
Requisito 2 do enunciado: arquivo de saida com as informacoes do grafo.

Gera numero de vertices, numero de arestas, grau minimo, maximo, medio e
mediana de grau, seguidos das informacoes das componentes conexas.
"""

from graph_utils import all_degrees, edge_count
from components import summary as components_summary


def median(values):
    """Mediana de uma lista de numeros (media dos dois centrais se par)."""
    if not values:
        return 0.0

    ordered = sorted(values)
    n = len(ordered)
    middle = n // 2

    if n % 2 == 1:
        return float(ordered[middle])
    return (ordered[middle - 1] + ordered[middle]) / 2.0


def graph_stats(graph, adj_list: bool = False):
    """Dicionario com as estatisticas exigidas no requisito 2."""
    degrees = all_degrees(graph, adj_list)
    n = len(graph)

    return {
        "vertices": n,
        "arestas": edge_count(graph, adj_list),
        "grau_minimo": min(degrees) if degrees else 0,
        "grau_maximo": max(degrees) if degrees else 0,
        "grau_medio": (sum(degrees) / n) if n else 0.0,
        "mediana_grau": median(degrees),
    }


def stats_text(graph, adj_list: bool = False):
    """Bloco de texto com as estatisticas, no formato do arquivo de saida."""
    s = graph_stats(graph, adj_list)
    return "\n".join([
        f"Numero de vertices: {s['vertices']}",
        f"Numero de arestas: {s['arestas']}",
        f"Grau minimo: {s['grau_minimo']}",
        f"Grau maximo: {s['grau_maximo']}",
        f"Grau medio: {s['grau_medio']:.4f}",
        f"Mediana de grau: {s['mediana_grau']:.1f}",
    ])


def write_output(graph, output_path, adj_list: bool = False):
    """Escreve o arquivo de saida completo exigido pelo requisito 2."""
    content = "\n".join([
        "=== INFORMACOES DO GRAFO ===",
        "",
        stats_text(graph, adj_list),
        "",
        "=== COMPONENTES CONEXAS ===",
        "",
        components_summary(graph, adj_list),
        "",
    ])

    with open(output_path, "w", encoding="utf-8") as file:
        file.write(content)

    return content


def write_search_tree(parents, levels, output_path, root):
    """
    Requisito 4: grava a arvore de busca (pai e nivel de cada vertice).
    `parents` e `levels` sao os vetores devolvidos por BFS/DFS com
    generate_tree=True, indexados em 0.
    """
    lines = [
        f"Raiz da busca: {root + 1}",
        "",
        "vertice pai nivel",
    ]

    for vertex in range(len(parents)):
        if levels[vertex] == -1 and vertex != root:
            continue  # vertice fora da componente da raiz

        parent = parents[vertex]
        parent_label = "-" if parent is None else str(parent + 1)
        lines.append(f"{vertex + 1} {parent_label} {levels[vertex]}")

    content = "\n".join(lines) + "\n"

    with open(output_path, "w", encoding="utf-8") as file:
        file.write(content)

    return content


if __name__ == "__main__":
    from pathlib import Path
    from graph import Graph

    example = Path(__file__).with_name("graph_exemplo.txt")
    g = Graph(example, adj_list=True)
    print(write_output(g.graph, Path(__file__).with_name("saida_exemplo.txt"), True))
