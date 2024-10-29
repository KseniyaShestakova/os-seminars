
typedef struct node_t {
    struct node_t* next;
    int value;
} node_t;

typedef struct lockfree_stack_t {
    _Atomic(node_t*) top;
} lockfree_stack_t;

int lockfree_stack_init(lockfree_stack_t* stack) {
    if (stack == NULL) {
        return 1;
    }
    
    atomic_init(&stack->top, NULL);
    return 0;
}

int lockfree_stack_push(lockfree_stack_t* stack, int value) {
    if (stack == NULL) {
        return 1;
    }

    node_t *new_node = (node_t*) calloc(1, sizeof(node_t));
    new_node->value = value;

    node_t *old_top = NULL;

    while (1) {
        node_t *old_top = stack->top;
        new_node->next = old_top;

        if (atomic_compare_exchange_weak(&stack->top, &old_top, new_node))
            break;
    }

    return 0;
}
