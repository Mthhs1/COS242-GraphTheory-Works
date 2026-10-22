#ifndef MY_STACK_H
#define MY_STACK_H

#include <stdbool.h>

/*
my_stack.py: pilha encadeada. Assim como no Python, tem sua propria
estrutura de no (campo `item`), independente da node.h.
*/

typedef struct StackNode {
    int item;
    struct StackNode *next;
} StackNode;

typedef struct Stack {
    StackNode *head;
} Stack;

/* stack = Stack() no Python */
void Stack_init(Stack *stack);

void Stack_push(Stack *stack, int item);
int Stack_pop(Stack *stack);
bool Stack_isEmpty(Stack *stack);
int Stack_length(Stack *stack);
int Stack_top(Stack *stack);

#endif
