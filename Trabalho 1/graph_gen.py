from node import Node

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
