#ifndef NODE_H
#define NODE_H

/*
node.py: no da lista encadeada, usado pela fila (my_queue) e pela
lista de adjacencia.
*/

typedef struct Node {
    int value;
    struct Node *next;
} Node;

/* Node(value, next_node=None) no Python */
Node *Node_create(int value, Node *next_node);

#endif
