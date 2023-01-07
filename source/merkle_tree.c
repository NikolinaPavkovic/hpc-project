#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <omp.h>

#define HASH_LEN 32

typedef struct Node
{
    u_char hash[HASH_LEN];
    struct Node *left;
    struct Node *right;
} Node;

Node *create_node(u_char *left, u_char *right)
{
    Node *node = malloc(sizeof(Node));
    u_char data[HASH_LEN * 2];
    memcpy(data, left, HASH_LEN);
    memcpy(data + HASH_LEN, right, HASH_LEN);
    SHA256(data, HASH_LEN * 2, node->hash);
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node *create_level(Node **nodes, int n)
{
    Node *level = malloc(sizeof(Node) * n / 2);
    for (int i = 0; i < n; i += 2)
    {
        level[i / 2] = *create_node(nodes[i]->hash, nodes[i + 1]->hash);
    }
    return level;
}

Node *create_tree(u_char **data, int n)
{
    Node **nodes = malloc(sizeof(Node *) * n);
    for (int i = 0; i < n; i++)
    {
        nodes[i] = malloc(sizeof(Node));
        memcpy(nodes[i]->hash, data[i], HASH_LEN);
        nodes[i]->left = NULL;
        nodes[i]->right = NULL;
    }
    while (n > 1)
    {
        Node *level = create_level(nodes, n);
        free(nodes);
        nodes = level;
        n /= 2;
    }

    Node *root = nodes[0];
    free(nodes);
    return root;
}

int main()
{
    u_char data[][HASH_LEN] = {
        {0x61},
        {0x62},
        {0x63},
        {0x64}};
    int n = sizeof(data) / HASH_LEN;
    printf("n = %d", n);

    for (int i = 0; i < 4; i++)
    {
        printf("%hhn", data[i]);
    }
    /*Node *root = create_tree((u_char **)data, n);
    
    free(root);*/
    return 0;
}