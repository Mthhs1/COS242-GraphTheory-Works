from pathlib import Path


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

class Graph():
    def __init__(self, archive,adj_list : bool = False):
        
        if adj_list:
            pass
        
        self.graph = create_matrix_graph(archive)
    
    
if __name__ == "__main__":
    graph_file = Path(__file__).resolve().parents[2] / "graph1.txt"
    graph = Graph(graph_file, adj_list=False)
    print(graph.graph)