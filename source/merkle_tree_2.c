#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <omp.h>

#define BLOCK_SIZE 1024
#define HASH_SIZE 32

typedef struct node
{
    unsigned char hash[HASH_SIZE];
    struct node *left;
    struct node *right;   
}node;

void hash_data(unsigned char *data, int size, unsigned char *output) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, size);
    SHA256_Final(output, &ctx);
}

void hash_data_new(unsigned char *left, unsigned char *right, unsigned char *output) {
    u_char data[HASH_SIZE * 2];
    memcpy(data, left, HASH_SIZE);
    memcpy(data + HASH_SIZE, right, HASH_SIZE);
    SHA256(data, HASH_SIZE * 2, output);
}

node *create_node(unsigned char *hash, struct node *left, struct node *right) {
    node *n = malloc(sizeof(node));
    memcpy(n->hash, hash, HASH_SIZE);
    n->left = left;
    n->right = right;
    return n;
}

node *create_tree(unsigned char **hashes, int count, int *counter) {
    if (count == 1) {
        return create_node(hashes[0], NULL, NULL);
    }

    printf("Broj novih cvorova: %d\n", sizeof(hashes)/sizeof(hashes[0]));

    int i;
    int j = 0;
    unsigned char **new_hashes = malloc(sizeof(unsigned char *) * (count+1)/2);
    for (i = 0; i < count - 1; i+=2)
    {
        unsigned char *left = hashes[i];
        unsigned char *right = hashes[i + 1];
        unsigned char *new_hash = malloc(HASH_SIZE);
        /*hash_data(left, HASH_SIZE, new_hash);
        hash_data(right, HASH_SIZE, new_hash);*/
        hash_data_new(left, right, new_hash);
        new_hashes[j++] = new_hash;
    }

    if(count % 2 == 1) {
        new_hashes[j++] = hashes[count-1];
    }

    *counter += 1;

    return create_tree(new_hashes, j, counter);
}

node *create_tree_new2(node **hashes, int count, int *counter) {
    if (count == 1) {
        return hashes[0];
    }

    printf("Broj novih cvorova: %d\n", sizeof(hashes)/sizeof(hashes[0]));

    int i;
    int j = 0;
    unsigned char **new_hashes = malloc(sizeof(node *) * (count+1)/2);
    for (i = 0; i < count - 1; i+=2)
    {
        /*unsigned char *left = hashes[i];
        unsigned char *right = hashes[i + 1];
        unsigned char *new_hash = malloc(HASH_SIZE);
        hash_data_new(left, right, new_hash);
        new_hashes[j++] = new_hash;*/
        node *node = malloc(sizeof(struct node));
        unsigned char *left = hashes[i];
        unsigned char *right = hashes[i + 1];
        hash_data_new(left, right, node->hash);
        node->left = left;
        node->right = right;
        new_hashes[j++] = node;
}

    if(count % 2 == 1) {
        new_hashes[j++] = hashes[count-1];
    }

    *counter += 1;

    return create_tree(new_hashes, j, counter);
}

node *create_tree_new(unsigned char **hashes, int count, int *counter) {
    int i;
    int j = 0;
    node **nodes = malloc(sizeof(node *) * (count+1)/2);
    for(int i = 0, j = 0; i < count; i += 2, j++) {
        if (i != count - 1) {
            node *node = malloc(sizeof(struct node));
            unsigned char *left = hashes[i];
            unsigned char *right = hashes[i + 1];
            hash_data_new(left, right, node->hash);
            node->left = left;
            node->right = right;
            nodes[j] = node;
        } else {
            nodes[j] = hashes[i];
        }
    }
}

void free_tree(node *root) {
    if(root == NULL) {
        return;
    }
    free_tree(root -> left);
    free_tree(root -> right);
    free(root);
}

void print_tree(node *root, int depth) {
    printf("USLO");
    if (root == NULL) {
        return;
    }

    printf("%d", *root->hash);

    printf("\n");
    print_tree(root->left, depth+1);
    printf(" ");
    print_tree(root->right, depth+1);

}

int count_nodes(int count){
    int depth;
    if (count % 2 == 1) {
        depth = count / 2 + 1;
    } else {
        depth = count / 2;
    }
    int num_of_nodes = 0;
    for(int i = 1; i <= depth; i++) {
        num_of_nodes += i;
    }
    return num_of_nodes;
}

int main() {
    unsigned char data[4][HASH_SIZE] = {
    {'h'},
    {'t'},
    {'f'},
    {'b'}
    };

    int size = sizeof(data) / HASH_SIZE;

    unsigned char *hashes[size];

    for(int i = 0; i < size; i++) {
        unsigned char *hash = malloc(HASH_SIZE);
        hash_data(data[i], HASH_SIZE, hash);
        hashes[i] = hash;
    }

    node *first_level_nodes[size];

    for(int i = 0; i < size; i++) {
        node *n = malloc(sizeof(node));
        hash_data(data[i], HASH_SIZE, n->hash);
        n->left = NULL;
        n->right = NULL;
        first_level_nodes[i] = n;
    }

    //printf("%d", sizeof(first_level_nodes)/sizeof(first_level_nodes[0]));

    int counter = 0;

    struct node *root = create_tree_new2(first_level_nodes, size, &counter);
    
    print_tree(root,1);


    /*unsigned char *hashes_proof[2];

    for(int i = 0; i < 4; i++) {
        unsigned char *hash = malloc(HASH_SIZE);
        hash_data(data[i], HASH_SIZE, hash);
        hashes_proof[i] = hash;
    }

    hash_data(root->hash, HASH_SIZE, hashes_proof);
    if (memcmp(hashes_proof, root->hash, HASH_SIZE) == 0) {
        printf("Data integrity verified\n");
    } else {
        printf("ERROR: Data has been modified\n");
    }*/

    free_tree(root);
    return 0;

}