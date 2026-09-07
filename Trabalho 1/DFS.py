import array
from my_stack import Stack

def DFS_adj_list(graph, start_node, generate_tree: bool = False):
    
    stack = Stack()
    vector = array.array('i', [0 for _ in range(len(graph))])
    result_discovered = []
    
    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    
    stack.push(start_node)
    
    if generate_tree:
        levels[start_node] = 0
    
    while not stack.isEmpty():
        
        current_node = stack.pop()
        
        if vector[current_node] == 0:
            result_discovered.append(current_node)
            vector[current_node] = 1
            
            neighbor = graph[current_node].next
            
            while neighbor != None:
                if vector[neighbor.value] == 0:
                    stack.push(neighbor.value)  
                    
                    if generate_tree:
                        parents[neighbor.value] = current_node
                        levels[neighbor.value] = levels[current_node] + 1
                        
                neighbor = neighbor.next
        
    if generate_tree:
        return result_discovered, parents, levels
    
    return result_discovered
    
    
def DFS_matrix(graph, start_node, generate_tree: bool = False):
    
    stack = Stack()
    vector = array.array('i', [0 for _ in range(len(graph))])
    result_discovered = []
    
    parents = [None] * len(graph)
    levels = [-1] * len(graph)
    
    stack.push(start_node)
    
    if generate_tree:
        levels[start_node] = 0
    
    while not stack.isEmpty():
        
        current_node = stack.pop()
        
        if vector[current_node] == 0:
            result_discovered.append(current_node)
            vector[current_node] = 1
            
            # Fazemos o Loop de trás para frente para que o nó com menor valor seja descoberto primeiro, já que a pilha é LIFO
            for neighbor in range(len(graph)-1,-1,-1):
                if graph[current_node][neighbor] == 1 and vector[neighbor] == 0:
                    stack.push(neighbor)
                    
                    if generate_tree:
                        parents[neighbor] = current_node
                        levels[neighbor] = levels[current_node] + 1
                        
    if generate_tree:
        return result_discovered, parents, levels
    
    return result_discovered
