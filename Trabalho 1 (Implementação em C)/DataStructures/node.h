#ifndef NODE_H
#define NODE_H

/*
node.py: no da lista encadeada, usado pela fila (my_queue) e pela
lista de adjacencia. Na Parte 2 ganhou o campo `weight`: o peso da
aresta na lista de adjacencia (os nos da fila nao o usam).
*/

typedef struct Node {
    int value;
    double weight; /* peso da aresta (Parte 2); 1.0 em grafos sem pesos */
    struct Node *next;
} Node;

/* Node(value, weight, next_node=None) no Python */
Node *Node_create(int value, double weight, Node *next_node);

#endif
