from pathlib import Path
from node import Node
from my_queue import Queue
from BFS import BFS_adj_list, BFS_matrix
from DFS import DFS_adj_list, DFS_matrix
import graph_gen

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
    
    def BFS(self, start_node, generate_tree : bool = False, target_mode : int = -1):
        
        if self.adj_list:
            return BFS_adj_list(self.graph,start_node, generate_tree, target_mode)
        
        return BFS_matrix(self.graph, start_node, generate_tree)
    
    def DFS(self, start_node, generate_tree : bool = False):
        if self.adj_list:
            return DFS_adj_list(self.graph, start_node, generate_tree)
        return DFS_matrix(self.graph, start_node, generate_tree)

    def Path(self, start_node, target_node):
        if self.adj_list:
            _, parents, _ = self.BFS(start_node, generate_tree=True, target_mode=target_node)

            if start_node == target_node:
                return [start_node]

            if parents[target_node] == None:
                return None

            i = target_node
            path = []
            while i != start_node:
                path.append(i)
                i = parents[i]
            
            return [i]+path[::-1]

        #return graph_gen.path_matrix(self.graph, start_node, target_node)

if __name__ == "__main__":
    graph_file = Path(__file__).resolve().parents[1] / "graph1.txt"
    print("\nGrafo 1 - Matriz de Adjacência")
    graph1 = Graph(graph_file)
    print(graph1)
    #print(graph1.BFS(1, generate_tree=True))
    print("DFS:")
    print(graph1.DFS(1))
    
    print("\nGrafo 1 - Lista de Adjacência")
    graph2 = Graph(graph_file, adj_list=True)
    print(graph2)
    #print("DFS:")
    #print(graph2.DFS(1, generate_tree=True))
    #print(graph2.BFS(1, generate_tree=True))
    print(graph2.Path(0,3))
    
    