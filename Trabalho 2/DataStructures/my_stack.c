#include <stdio.h>
#include <stdlib.h>

#include "my_stack.h"
#include "graph_utils.h"

void Stack_init(Stack *stack) {
    stack->head = NULL;
}

void Stack_push(Stack *stack, int item) {
    StackNode *node = xmalloc(sizeof(StackNode));
    node->item = item;
    node->next = NULL;

    if (stack->head == NULL) {
        stack->head = node;
        return;
    }

    StackNode *bottom = stack->head;
    stack->head = node;
    stack->head->next = bottom;
}

int Stack_pop(Stack *stack) {
    StackNode *popped = stack->head;
    stack->head = stack->head->next;
    int item = popped->item;
    free(popped); /* em C o no removido precisa ser liberado */
    return item;
}

bool Stack_isEmpty(Stack *stack) {
    return stack->head == NULL;
}

int Stack_length(Stack *stack) {
    int n = 0;

    if (Stack_isEmpty(stack)) {
        return n;
    }

    StackNode *actual = stack->head;

    while (actual != NULL) {
        n += 1;
        actual = actual->next;
    }

    return n;
}

int Stack_top(Stack *stack) {
    return stack->head->item;
}

#ifdef MY_STACK_MAIN
/* if __name__ == "__main__" do my_stack.py */
int main(void) {
    Stack stack1;
    Stack_init(&stack1);
    Stack_push(&stack1, 1);
    Stack_push(&stack1, 2);
    printf("%d\n", Stack_pop(&stack1));
    printf("%s\n", Stack_isEmpty(&stack1) ? "True" : "False");
    printf("%d\n", Stack_length(&stack1));
    printf("%d\n", Stack_top(&stack1));
    return 0;
}
#endif
