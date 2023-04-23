#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <omp.h>

#define HASH_SIZE 32
#define COUNT 10
#define MAX_NUM_OF_CHILDREN 3

typedef struct Node
{
    unsigned char hash[HASH_SIZE];
    struct Node *children[MAX_NUM_OF_CHILDREN];
} Node;

void hash_data(unsigned char *data, int size, unsigned char *output) {
    SHA256(data, HASH_SIZE, output);
}

void hash_data_new(Node **nodes, unsigned char *output, int count) {
    u_char data[HASH_SIZE * count];
    for (int i = 0; i < count; i++) {
        memcpy(data + HASH_SIZE*i, nodes[i], HASH_SIZE);
    }
    SHA256(data, HASH_SIZE * count, output);
}

Node *return_if_less_than_max_left(Node **nodes, int count) {
    int j = 0;
    
    Node *node = malloc(sizeof(Node));
    Node *hash_nodes[MAX_NUM_OF_CHILDREN];

    for(int i = 0; i < count; i++) {
        hash_nodes[i] = nodes[i];
    }

    hash_data_new(hash_nodes, node->hash, count);

    for(int i = 0; i < count; i++) {
        node->children[i] = nodes[i];
    }

    return node;
}

Node *create_non_binary_tree_sequential(Node **nodes, int count) {
    if(count == 1) {
        return nodes[0];
    } else if (count < MAX_NUM_OF_CHILDREN) {
        return return_if_less_than_max_left(nodes, count);
    }

    int i;
    int j = 0;
    Node **new_nodes = malloc(sizeof(Node *) * (count + 2)/MAX_NUM_OF_CHILDREN);
    
    for(i = 0; i <= count - MAX_NUM_OF_CHILDREN; i += MAX_NUM_OF_CHILDREN) {
        Node *node = malloc(sizeof(Node));
        Node *hash_nodes[MAX_NUM_OF_CHILDREN];
        
        for(int u = 0; u < MAX_NUM_OF_CHILDREN; u++) {
            hash_nodes[u] = nodes[i+u];
        }
        hash_data_new(hash_nodes, node->hash, MAX_NUM_OF_CHILDREN);
        
        for(int u = 0; u < MAX_NUM_OF_CHILDREN; u++) {
            node->children[u] = nodes[i+u];
        }
        new_nodes[j++] = node;
        
    }
    
    if(count % MAX_NUM_OF_CHILDREN != 0) {
        for(int u = count % MAX_NUM_OF_CHILDREN; u > 0; u--) {
            new_nodes[j++] = nodes[count - u];
        }
    }

    printf("\n\n");

    return create_non_binary_tree_sequential(new_nodes, j);
}

Node *create_non_binary_tree_parallel(Node **nodes, int count) {
    if(count == 1) {
        return nodes[0];
    } else if (count < MAX_NUM_OF_CHILDREN) {
        return return_if_less_than_max_left(nodes, count);
    }

    int i;
    int j = 0;
    Node **new_nodes = malloc(sizeof(Node *) * (count + 2)/MAX_NUM_OF_CHILDREN);
    
    #pragma omp parallel for ordered
    for(i = 0; i <= count - MAX_NUM_OF_CHILDREN; i += MAX_NUM_OF_CHILDREN) {
        Node *node = malloc(sizeof(Node));
        Node *hash_nodes[MAX_NUM_OF_CHILDREN];
        
        for(int u = 0; u < MAX_NUM_OF_CHILDREN; u++) {
            hash_nodes[u] = nodes[i+u];
        }
        hash_data_new(hash_nodes, node->hash, MAX_NUM_OF_CHILDREN);
        
        for(int u = 0; u < MAX_NUM_OF_CHILDREN; u++) {
            node->children[u] = nodes[i+u];
        }

        #pragma omp ordered
        {
            new_nodes[j++] = node;
        }
    }
    
    if(count % MAX_NUM_OF_CHILDREN != 0) {
        for(int u = count % MAX_NUM_OF_CHILDREN; u > 0; u--) {
            new_nodes[j++] = nodes[count - u];
        }
    }

    printf("\n\n");

    return create_non_binary_tree_parallel(new_nodes, j);
}

void free_tree(Node *root) {
    if(root == NULL) {
        return;
    }

    for(int i = 0; i < MAX_NUM_OF_CHILDREN; i++) {
        free_tree(root->children[i]);
    }

    free(root);
}

void print_tree(Node *root, int space) {
    if (root == NULL) {
        return;
    }

    space += COUNT;

    print_tree(root->children[0], space);

    printf("\n");
    for (int j = COUNT; j < space; j++){
        printf(" ");
    }
    printf("%d\n", *root->hash);

    for (int i = 1; i < MAX_NUM_OF_CHILDREN; i++) {
        print_tree(root->children[i], space);
    }
    
}

int main() {
    unsigned char data[][HASH_SIZE] = {
    {'a'}, {'b'}, {'c'}, {'m'}, {'n'}, {'a'}, {'b'}, {'c'}, {'a'}, {'b'}, {'c'}, {'k'}, {'a'}, {'b'}, {'c'}, {'m'}, {'n'}, {'a'}, {'b'}, {'c'}, {'a'}, {'b'}, {'c'}, {'k'},
    {'a'}, {'b'}, {'c'}, {'m'}, {'n'}, {'a'}, {'b'}, {'c'}, {'a'}, {'b'}, {'c'}, {'k'}, {'a'}, {'b'}, {'c'}, {'m'}, {'n'}, {'a'}, {'b'}, {'c'}, {'a'}, {'b'}, {'c'}, {'k'},

    };

    int size = sizeof(data) / HASH_SIZE;

    Node *first_level_nodes[size];

    for(int i = 0; i < size; i++) {
        Node *node = malloc(sizeof(Node));
        hash_data(data[i], HASH_SIZE, node->hash);
        for(int j = 0; j < MAX_NUM_OF_CHILDREN; j++) {
            node->children[j] = NULL;
        }
        first_level_nodes[i] = node;
    }

    double end_seq, start_seq = omp_get_wtime();
    Node *root_seq = create_non_binary_tree_sequential(first_level_nodes, size);
    end_seq = omp_get_wtime();

    printf("Trajanje sekvencijalnog koda: %lf\n\n", end_seq - start_seq);

    double end, start = omp_get_wtime();
    Node *root = create_non_binary_tree_parallel(first_level_nodes, size);
    end = omp_get_wtime();

    printf("Trajanje paralelnog koda: %lf\n\n", end - start);

    //print_tree(root, 0);

    //verification
    unsigned char data_proof[][HASH_SIZE] = {
    {'a'}, {'b'}, {'c'}
    };

    Node *first_level_nodes_proof[size];

    for(int i = 0; i < size; i++) {
        Node *node = malloc(sizeof(Node));
        hash_data(data_proof[i], HASH_SIZE, node->hash);
        for(int j = 0; j < MAX_NUM_OF_CHILDREN; j++) {
            node->children[j] = NULL;
        }
        first_level_nodes_proof[i] = node;
    }

    Node *new_root = create_non_binary_tree_parallel(first_level_nodes_proof, size);

    if (memcmp(new_root->hash, root->hash, HASH_SIZE) == 0) {
        printf("verified\n");
    } else {
        printf("data has been modified\n");
    }

    free_tree(root);
    return 0;
}

