from pathlib import Path
from node import Node
from my_queue import Queue
import array

def create_empty_list_graph(n):
    return [Node(None, None) for _ in range(n)]

def create_list_graph(archive_directory):
    with open(f"{archive_directory}", "r") as file:
        line = file.readline()
        n = int(line)
        graph = create_empty_list_graph(n)
        last_nodes = [None for _ in range(n)]

        for i in range(n):
            line = file.readline()
            line = line.strip().split()

            a = int(line[0]) - 1
            b = int(line[1]) - 1
            
            node_a = Node(b, None)
            node_b = Node(a, None)
            
            if last_nodes[a] == None:
                graph[a].next = node_a
            else:
                last_nodes[a].next = node_a

            if last_nodes[b] == None:
                graph[b].next = node_b
            else:
                last_nodes[b].next = node_b

            last_nodes[a] = last_nodes[a].next if last_nodes[a] != None else graph[a].next
            last_nodes[b] = last_nodes[b].next if last_nodes[b] != None else graph[b].next

        return graph

def create_empty_matrix_graph(n):
    return [[0 for _ in range(n)] for _ in range(n)]

def create_matrix_graph(archive_directory):
    
    with open(f"{archive_directory}", "r") as file:
        line = file.readline()
        n = int(line)
        graph = create_empty_matrix_graph(n)

        for i in range(n):
            line = file.readline()
            line = line.strip().split()

            a = int(line[0]) - 1
            b = int(line[1]) - 1
            
            graph[a][b] = 1
            graph[b][a] = 1

        return graph

def str_adj_list(graph):
    result = ""
    for i in range(len(graph)):
        result += f"{i + 1}: "
        current_node = graph[i].next
        while current_node != None:
            result += f"{current_node.value + 1} -> "
            current_node = current_node.next
        result += "None\n"
    return result

def str_matrix(graph):
    result = ""
    for i in range(len(graph)):
        result += str(graph[i]) + "\n"
    return result

def BFS_adj_list(graph, start_node, generate_tree: bool = False):
    queue = Queue()
    vector = array.array('i', [0 for _ in range(len(graph))])

    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    queue.enqueue(start_node)
    vector[start_node] = 1
    result_discovered = []
    
    if generate_tree:
        levels[start_node] = 0
        
    while not queue.isEmpty():
        current_node = queue.dequeue()
        result_discovered.append(current_node)
        
        neighbor = graph[current_node].next
        
        while neighbor != None:
            if vector[neighbor.value] == 0:
                queue.enqueue(neighbor.value)
                vector[neighbor.value] = 1
                
                if generate_tree:
                    parents[neighbor.value] = current_node
                    levels[neighbor.value] = levels[current_node] + 1
                    
            neighbor = neighbor.next
            
    if generate_tree:
        return result_discovered, parents, levels
        
    return result_discovered

class Graph():
    def __init__(self, archive,adj_list : bool = False):
        
        self.adj_list = adj_list
        
        if adj_list:
            self.graph = create_list_graph(archive)
            return
        
        self.graph = create_matrix_graph(archive)
        return
    
    def __str__(self):
        
        if self.adj_list:
            return str_adj_list(self.graph)
        
        return str_matrix(self.graph)
    
    def BFS(self, start_node, generate_tree : bool = False):
        
        
        if self.adj_list:
            return BFS_adj_list(self.graph,start_node, generate_tree)
        
        return self.BFS_matrix(start_node)
    
if __name__ == "__main__":
    graph_file = Path(__file__).resolve().parents[2] / "graph1.txt"
    graph1 = Graph(graph_file, adj_list=False)
    print(graph1)
    
    graph2 = Graph(graph_file, adj_list=True)
    print(graph2.BFS(1, generate_tree=True))
    print(graph2)
    
    
    
    