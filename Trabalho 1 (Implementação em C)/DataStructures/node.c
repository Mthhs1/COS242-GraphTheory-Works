#include <stdlib.h>

#include "node.h"
#include "graph_utils.h"

Node *Node_create(int value, Node *next_node) {
    Node *node = xmalloc(sizeof(Node));
    node->value = value;
    node->next = next_node;
    return node;
}
