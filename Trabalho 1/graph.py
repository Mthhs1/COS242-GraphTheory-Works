from pathlib import Path
from node import Node
from my_queue import Queue
from BFS import BFS_adj_list, BFS_matrix
from DFS import DFS_adj_list, DFS_matrix
import graph_gen
import components
import distance as distance_mod
import graph_stats

class Graph():
    def __init__(self, archive,adj_list : bool = False):
        
        self.adj_list = adj_list
        
        if adj_list:
            self.graph = graph_gen.create_list_graph(archive)
            return
        
        self.graph = graph_gen.create_matrix_graph(archive)
        return
    
    def __str__(self):
        
        if self.adj_list:
            return graph_gen.str_adj_list(self.graph)
        
        return graph_gen.str_matrix(self.graph)
    
    def BFS(self, start_node, generate_tree : bool = False):
        
        if self.adj_list:
            return BFS_adj_list(self.graph,start_node, generate_tree)
        
        return BFS_matrix(self.graph, start_node, generate_tree)
    
    def DFS(self, start_node, generate_tree : bool = False):
        if self.adj_list:
            return DFS_adj_list(self.graph, start_node, generate_tree)
        return DFS_matrix(self.graph, start_node, generate_tree)

    # ------------------------------------------------------------------
    # Requisitos 2, 5 e 6 do enunciado.
    # ------------------------------------------------------------------

    def connected_components(self):
        """Componentes conexas, em ordem decrescente de tamanho (rotulos 1-based)."""
        return components.connected_components(self.graph, self.adj_list)

    def distance(self, u, v):
        """Distancia entre os vertices u e v (rotulos 1-based). -1 se nao houver caminho."""
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
        """Requisito 4: grava pai e nivel de cada vertice da arvore de busca."""
        search = self.DFS if use_dfs else self.BFS
        _, parents, levels = search(start_node, generate_tree=True)
        return graph_stats.write_search_tree(parents, levels, output_path, start_node)


if __name__ == "__main__":
    graph_file = Path(__file__).resolve().parents[2] / "graph1.txt"
    print("\nGrafo 1 - Matriz de Adjacência")
    graph1 = Graph(graph_file)
    print(graph1)
    #print(graph1.BFS(1, generate_tree=True))
    print("DFS:")
    print(graph1.DFS(1))
    
    print("\nGrafo 1 - Lista de Adjacência")
    graph2 = Graph(graph_file, adj_list=True)
    print(graph2)
    print("DFS:")
    print(graph2.DFS(1, generate_tree=True))
    #print(graph2.BFS(1, generate_tree=True))
    
    
    
    