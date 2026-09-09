from node import Node

def create_empty_list_graph(n):
    return [Node(None, None) for _ in range(n)]

def create_list_graph(archive_directory):
    with open(f"{archive_directory}", "r") as file:
        line = file.readline()
        n = int(line)
        graph = create_empty_list_graph(n)
        last_nodes = [None for _ in range(n)]
        neighbor = {i: [] for i in range(n)}

        # CORRECAO: o arquivo tem n vertices na 1a linha e depois m arestas,
        # uma por linha. O laco antigo (range(n)) lia n linhas de aresta, o
        # que so funciona quando m == n. Agora lemos ate o fim do arquivo.
        for line in file:
            parts = line.strip().split()
            if len(parts) < 2:
                continue

            a = int(parts[0]) - 1
            b = int(parts[1]) - 1

            neighbor[a].append(b)
            neighbor[b].append(a)


        for i in neighbor:
            neighbor[i].sort(reverse=True)
            
            for j in range(len(neighbor[i])):
                if last_nodes[i] == None:
                    graph[i].next = Node(neighbor[i][j], None)
                    last_nodes[i] = graph[i].next
                else:
                    last_nodes[i].next = Node(neighbor[i][j], None)
                    last_nodes[i] = last_nodes[i].next

        return graph      

def create_empty_matrix_graph(n):
    return [[0 for _ in range(n)] for _ in range(n)]

def create_matrix_graph(archive_directory):
    
    with open(f"{archive_directory}", "r") as file:
        line = file.readline()
        n = int(line)
        graph = create_empty_matrix_graph(n)

        # CORRECAO: o arquivo tem n vertices na 1a linha e depois m arestas,
        # uma por linha. O laco antigo (range(n)) lia n linhas de aresta, o
        # que so funciona quando m == n. Agora lemos ate o fim do arquivo.
        for line in file:
            parts = line.strip().split()
            if len(parts) < 2:
                continue

            a = int(parts[0]) - 1
            b = int(parts[1]) - 1
            
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
