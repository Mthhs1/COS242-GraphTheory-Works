#include <stdlib.h>

#include "node.h"
#include "graph_utils.h"

Node *Node_create(int value, double weight, Node *next_node) {
    Node *node = xmalloc(sizeof(Node));
    node->value = value;
    node->weight = weight;
    node->next = next_node;
    return node;
}
