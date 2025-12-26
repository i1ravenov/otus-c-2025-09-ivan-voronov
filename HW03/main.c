#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    long value;
    struct Node *next;
} Node;

void print_int(long x) {
    printf("%ld ", x);
    fflush(NULL);
}

long p(long x) {
    return x & 1;
}

Node *add_element(long value, Node *next) {
    Node *n = malloc(sizeof(Node));
    if (!n) abort();
    n->value = value;
    n->next = next;
    return n;
}

void m(Node *list, void (*fn)(long)) {
    if (!list) return;

    fn(list->value);
    m(list->next, fn);
}

Node *f(Node *list, Node *acc, long (*pred)(long)) {
    if (!list) return acc;

    if (pred(list->value)) {
        acc = add_element(list->value, acc);
    }

    return f(list->next, acc, pred);
}

void free_list(Node *list) {
    while (list) {
        Node *next = list->next;
        free(list);
        list = next;
    }
}

int main(void) {
    long data[] = {4, 8, 15, 16, 23, 42};
    size_t data_length = sizeof(data) / sizeof(data[0]);

    Node *list = NULL;

    for (size_t i = data_length; i > 0; i--) {
        list = add_element(data[i - 1], list);
    }

    m(list, print_int);
    puts("");

    Node *filtered = f(list, NULL, p);

    m(filtered, print_int);
    puts("");

    free_list(filtered);
    free_list(list);

    return 0;
}
