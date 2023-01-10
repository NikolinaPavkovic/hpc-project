#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <omp.h>

#define BLOCK_SIZE 1024
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

void hash_data_new(Node **nodes, unsigned char *output) {
    u_char data[HASH_SIZE * MAX_NUM_OF_CHILDREN];
    for (int i = 0; i < MAX_NUM_OF_CHILDREN; i++) {
        memcpy(data + HASH_SIZE*i, nodes[i], HASH_SIZE);
    }
    SHA256(data, HASH_SIZE * MAX_NUM_OF_CHILDREN, output);
}

Node *return_if_less_than_max_left(Node **nodes, int count) {
    int j = 0;
    
    Node *node = malloc(sizeof(Node));
    Node *hash_nodes[MAX_NUM_OF_CHILDREN];

    for(int i = 0; i < count; i++) {
        hash_nodes[i] = nodes[i];
    }

    hash_data_new(hash_nodes, node->hash);

    for(int i = 0; i < count; i++) {
        node->children[i] = nodes[i];
    }

    //printf(" Node: %d\n", *node->hash);

    return node;
}

Node *create_non_binary_tree(Node **nodes, int count) {
    //printf("USLO ");
    if(count == 1) {
        return nodes[0];
    } else if (count < MAX_NUM_OF_CHILDREN) {
        //printf("TWO LEFT \n");
        return return_if_less_than_max_left(nodes, count);
    }

    int j = 0;
    Node **new_nodes = malloc(sizeof(Node *) * (count+2)/MAX_NUM_OF_CHILDREN);
    
    #pragma omp parallel for ordered
    for(int i = 0; i < count - MAX_NUM_OF_CHILDREN + 1; i+=MAX_NUM_OF_CHILDREN) {
        Node *node = malloc(sizeof(Node));
        Node *hash_nodes[MAX_NUM_OF_CHILDREN];
        
        for(int u = 0; u < MAX_NUM_OF_CHILDREN; u++) {
            hash_nodes[u] = nodes[i+u];
        }
        hash_data_new(hash_nodes, node->hash);
        
        for(int u = 0; u < MAX_NUM_OF_CHILDREN; u++) {
            node->children[u] = nodes[i+u];
        }
        new_nodes[j++] = node;
    }

    //TODO: srediti ovo
    if(count % 3 == 1) {
        printf("OKI\n");
        new_nodes[j++] = nodes[count - 1];
    } else if (count % 3 == 2) {
        new_nodes[j++] = nodes[count - 2];
        new_nodes[j++] = nodes[count - 1];
    }

    printf("\n\n");

    return create_non_binary_tree(new_nodes, j);
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
    {'a'},
    {'b'},
    {'c'},
    {'d'},
    {'e'},
    {'f'},
    {'g'},
    {'h'},
    {'i'},
    {'j'},
    {'k'},
    {'l'},
    {'m'},
    {'n'}
    };

    int size = sizeof(data) / HASH_SIZE;

    Node *first_level_nodes[size];

    for(int i = 0; i < size; i++) {
        Node *node = malloc(sizeof(Node));
        hash_data(data[i], HASH_SIZE, node->hash);
        for(int j = 0; j < MAX_NUM_OF_CHILDREN; j++) {
            node->children[j] = NULL;
            //printf("Child %d: %d\n", j, node->children[j]);
        }
        first_level_nodes[i] = node;
        //printf("Hash: %d\n", *first_level_nodes[i]->hash);
    }

    Node *root = create_non_binary_tree(first_level_nodes, size);

    print_tree(root, 0);

    free_tree(root);
    
}

