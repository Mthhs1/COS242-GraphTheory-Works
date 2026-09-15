from pathlib import Path
from BFS import BFS_adj_list, BFS_matrix
from DFS import DFS_adj_list, DFS_matrix
import graph_gen
import components
import distance as distance_mod
import graph_stats


class Graph():
    def __init__(self, archive, adj_list: bool = False):
        """
        Carrega o grafo do arquivo `archive`. adj_list=True usa lista de
        adjacencia; adj_list=False (padrao) usa matriz de adjacencia.
        """
        self.adj_list = adj_list

        if adj_list:
            self.graph = graph_gen.create_list_graph(archive)
            return

        self.graph = graph_gen.create_matrix_graph(archive)

    def _to_index(self, label):
        """Converte rotulo 1-based (como no arquivo de entrada) em indice
        0-based, validando os limites."""
        index = label - 1
        if not 0 <= index < len(self.graph):
            raise ValueError(
                f"vertice invalido: {label} (o grafo tem {len(self.graph)} vertices)"
            )
        return index

    def __str__(self):

        if self.adj_list:
            return graph_gen.str_adj_list(self.graph)

        return graph_gen.str_matrix(self.graph)

    def BFS(self, start_node, generate_tree: bool = False, target=None):
        """
        Busca em largura a partir de `start_node` (rotulo 1-based).

        Com generate_tree=False devolve apenas a ordem de descoberta; com
        True devolve a tripla (ordem, pais, niveis), tudo indexado em 0.
        Se `target` (rotulo 1-based) for fornecido, a busca encerra ao
        alcancar o alvo, em ambas as representacoes.
        """
        start = self._to_index(start_node)
        target_index = None if target is None else self._to_index(target)

        if self.adj_list:
            order, parents, levels = BFS_adj_list(self.graph, start, target_index)
        else:
            order, parents, levels = BFS_matrix(self.graph, start, target_index)

        if generate_tree:
            return order, parents, levels
        return order

    def DFS(self, start_node, generate_tree: bool = False):
        """
        Busca em profundidade a partir de `start_node` (rotulo 1-based).

        Mesmo contrato de retorno da BFS: apenas a ordem, ou a tripla
        (ordem, pais, niveis) quando generate_tree=True.
        """
        start = self._to_index(start_node)

        if self.adj_list:
            order, parents, levels = DFS_adj_list(self.graph, start)
        else:
            order, parents, levels = DFS_matrix(self.graph, start)

        if generate_tree:
            return order, parents, levels
        return order

    # ------------------------------------------------------------------
    # Requisitos 2, 5 e 6 do enunciado.
    # ------------------------------------------------------------------

    def connected_components(self):
        """Componentes conexas, em ordem decrescente de tamanho (rotulos 1-based)."""
        return components.connected_components(self.graph, self.adj_list)

    def distance(self, u, v):
        """Distancia entre os vertices u e v (rotulos 1-based). -1 se nao houver caminho."""
        self._to_index(u)
        self._to_index(v)
        return distance_mod.distance(self.graph, u, v, self.adj_list)

    def diameter(self, approximate: bool = False):
        """Diametro do grafo. approximate=True usa a heuristica de dupla varredura."""
        if approximate:
            return distance_mod.approximate_diameter(self.graph, self.adj_list)
        return distance_mod.diameter(self.graph, self.adj_list)

    def stats(self):
        """n, m, grau minimo/maximo/medio e mediana de grau."""
        return graph_stats.graph_stats(self.graph, self.adj_list)

    def write_output(self, output_path):
        """Requisito 2: arquivo de saida com estatisticas e componentes conexas."""
        return graph_stats.write_output(self.graph, output_path, self.adj_list)

    def write_search_tree(self, output_path, start_node, use_dfs: bool = False):
        """Requisito 4: grava pai e nivel de cada vertice da arvore de busca
        (start_node em rotulo 1-based)."""
        search = self.DFS if use_dfs else self.BFS
        _, parents, levels = search(start_node, generate_tree=True)
        return graph_stats.write_search_tree(parents, levels, output_path, start_node - 1)


if __name__ == "__main__":
    graph_file = Path(__file__).resolve().parents[1] / "graph1.txt"

    print("\nGrafo 1 - Matriz de Adjacência")
    graph1 = Graph(graph_file)
    print(graph1)
    print("BFS a partir do vertice 1:", graph1.BFS(1))
    print("DFS a partir do vertice 1:", graph1.DFS(1))

    print("\nGrafo 1 - Lista de Adjacência")
    graph2 = Graph(graph_file, adj_list=True)
    print(graph2)
    print("BFS a partir do vertice 1:", graph2.BFS(1))
    print("DFS a partir do vertice 1:", graph2.DFS(1))
    print("Distancia entre os vertices 1 e 4:", graph2.distance(1, 4))
