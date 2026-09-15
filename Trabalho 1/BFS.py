from my_queue import Queue
import array


def BFS_adj_list(graph, start_node, target=None):
    """
    BFS em lista de adjacencia a partir de `start_node` (indice 0-based).

    Sempre devolve (ordem de descoberta, pais, niveis), indexados em 0.
    Se `target` (indice 0-based) for alcancado, a busca encerra assim que
    o alvo sai da fila; vertices ainda nao visitados ficam com pai None
    e nivel -1.
    """
    queue = Queue()
    vector = array.array('i', [0 for _ in range(len(graph))])

    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    levels[start_node] = 0

    queue.enqueue(start_node)
    vector[start_node] = 1
    result_discovered = []

    while not queue.isEmpty():
        current_node = queue.dequeue()
        result_discovered.append(current_node)

        if current_node == target:
            break

        neighbor = graph[current_node].next

        while neighbor != None:
            if vector[neighbor.value] == 0:
                queue.enqueue(neighbor.value)
                vector[neighbor.value] = 1
                parents[neighbor.value] = current_node
                levels[neighbor.value] = levels[current_node] + 1

            neighbor = neighbor.next

    return result_discovered, parents, levels


def BFS_matrix(graph, start_node, target=None):
    """
    BFS em matriz de adjacencia. Mesmo contrato de BFS_adj_list:
    sempre devolve (ordem de descoberta, pais, niveis), indexados em 0.
    """
    queue = Queue()
    vector = array.array('i', [0 for _ in range(len(graph))])

    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    levels[start_node] = 0

    queue.enqueue(start_node)
    vector[start_node] = 1
    result_discovered = []

    while not queue.isEmpty():
        current_node = queue.dequeue()
        result_discovered.append(current_node)

        if current_node == target:
            break

        for neighbor in range(len(graph)):
            if graph[current_node][neighbor] == 1 and vector[neighbor] == 0:
                queue.enqueue(neighbor)
                vector[neighbor] = 1
                parents[neighbor] = current_node
                levels[neighbor] = levels[current_node] + 1

    return result_discovered, parents, levels
