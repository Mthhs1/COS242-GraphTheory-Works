import array
from my_stack import Stack


def DFS_adj_list(graph, start_node):
    """
    DFS em lista de adjacencia a partir de `start_node` (indice 0-based).

    Sempre devolve (ordem de descoberta, pais, niveis), indexados em 0.
    Vertices nao alcancados ficam com pai None e nivel -1.
    """
    stack = Stack()
    vector = array.array('i', [0 for _ in range(len(graph))])
    result_discovered = []

    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    levels[start_node] = 0

    stack.push(start_node)

    while not stack.isEmpty():

        current_node = stack.pop()

        if vector[current_node] == 0:
            result_discovered.append(current_node)
            vector[current_node] = 1

            neighbor = graph[current_node].next

            while neighbor != None:
                if vector[neighbor.value] == 0:
                    stack.push(neighbor.value)
                    parents[neighbor.value] = current_node
                    levels[neighbor.value] = levels[current_node] + 1

                neighbor = neighbor.next

    return result_discovered, parents, levels


def DFS_matrix(graph, start_node):
    """
    DFS em matriz de adjacencia. Mesmo contrato de DFS_adj_list:
    sempre devolve (ordem de descoberta, pais, niveis), indexados em 0.

    O loop de tras para frente faz o vizinho de menor rotulo sair primeiro
    da pilha (LIFO), reproduzindo a ordem da versao em lista.
    """
    stack = Stack()
    vector = array.array('i', [0 for _ in range(len(graph))])
    result_discovered = []

    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    levels[start_node] = 0

    stack.push(start_node)

    while not stack.isEmpty():

        current_node = stack.pop()

        if vector[current_node] == 0:
            result_discovered.append(current_node)
            vector[current_node] = 1

            for neighbor in range(len(graph) - 1, -1, -1):
                if graph[current_node][neighbor] == 1 and vector[neighbor] == 0:
                    stack.push(neighbor)
                    parents[neighbor] = current_node
                    levels[neighbor] = levels[current_node] + 1

    return result_discovered, parents, levels
