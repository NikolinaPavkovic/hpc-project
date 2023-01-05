#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

typedef struct Node {
    u_char hash[32];
    struct Node *left;
    struct Node *right;
} Node;

int main() {
    printf("Hello!");
}