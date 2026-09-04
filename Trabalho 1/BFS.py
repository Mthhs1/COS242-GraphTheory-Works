from my_queue import Queue
import array


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

def BFS_matrix(graph, start_node, generate_tree: bool = False):
    
    queue = Queue()
    vector = array.array('i', [0 for _ in range(len(graph))])
    
    queue.enqueue(start_node)
    vector[start_node] = 1
    
    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    
    if generate_tree:
        levels[start_node] = 0
    
    result_discovered = []
    
    while queue.isEmpty() == False:
        current_node = queue.dequeue()
        result_discovered.append(current_node)
        
        for neighbor in range(len(graph)):
            if graph[current_node][neighbor] == 1 and vector[neighbor] == 0:
                queue.enqueue(neighbor)
                vector[neighbor] = 1
                
                if generate_tree:
                    parents[neighbor] = current_node
                    levels[neighbor] = levels[current_node] + 1
    
    if generate_tree:
        return result_discovered, parents, levels
    
    return result_discovered
