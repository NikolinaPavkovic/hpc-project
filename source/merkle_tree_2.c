#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <omp.h>

#define BLOCK_SIZE 1024
#define HASH_SIZE 32
#define COUNT 10

typedef struct node
{
    unsigned char hash[HASH_SIZE];
    struct node *left;
    struct node *right;   
}node;

void hash_data(unsigned char *data, int size, unsigned char *output) {
    /*SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, size);
    SHA256_Final(output, &ctx);*/
    SHA256(data, HASH_SIZE, output);
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

/*node *create_tree(unsigned char **hashes, int count, int *counter) {
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
        //hash_data(left, HASH_SIZE, new_hash);
        //hash_data(right, HASH_SIZE, new_hash);
        hash_data_new(left, right, new_hash);
        new_hashes[j++] = new_hash;
    }

    if(count % 2 == 1) {
        new_hashes[j++] = hashes[count-1];
    }

    *counter += 1;

    return create_tree(new_hashes, j, counter);
}*/

node *create_tree_new2(node **nodes, int count, int *counter) {
    if (count == 1) {
        return nodes[0];
    }

    /*printf("Broj novih cvorova: %d\n", sizeof(nodes)/sizeof(nodes[0]));
    fflush(stdout);*/

    int i;
    int j = 0;
    unsigned char **new_nodes = malloc(sizeof(node *) * (count+1)/2);
    for (i = 0; i < count - 1; i+=2)
    {
        node *node = malloc(sizeof(struct node));
        unsigned char *left = nodes[i];
        unsigned char *right = nodes[i + 1];
        hash_data_new(left, right, node->hash);
        node->left = left;
        node->right = right;
        new_nodes[j++] = node;
    }

    if(count % 2 == 1) {
        new_nodes[j++] = nodes[count-1];
    }

    *counter += 1;

    return create_tree_new2(new_nodes, j, counter);
}

void free_tree(node *root) {
    if(root == NULL) {
        return;
    }
    free_tree(root -> left);
    free_tree(root -> right);
    free(root);
}

void print_tree(node *root, int space) {
    //printf("USLO\n");
    if (root == NULL) {
        return;
    }

    space += COUNT;

    //printf("\n");
    print_tree(root->right, space);

    printf("\n");
    for (int i = COUNT; i < space; i++)
        printf(" ");
    printf("%d\n", *root->hash);
 
    //printf(" ");
    print_tree(root->left, space);

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
    unsigned char data[][HASH_SIZE] = {
    {'h'},
    {'t'},
    {'f'},
    {'b'},
    {'f'},
    {'f'},
    {'b'}
    };

    int size = sizeof(data) / HASH_SIZE;

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

    node *root = create_tree_new2(first_level_nodes, size, &counter);
    
    print_tree(root,0);

    //verification

    unsigned char data_proof[][HASH_SIZE] = {
    {'h'},
    {'t'},
    {'f'},
    {'b'},
    {'f'},
    {'f'},
    {'b'},
    {'f'}
    };


    node *first_level_nodes_proof[size];

    for(int i = 0; i < size; i++) {
        unsigned char *hash = malloc(HASH_SIZE);
        hash_data(data_proof[i], HASH_SIZE, hash);
        first_level_nodes_proof[i] = hash;
    }

    node *new_root = create_tree_new2(first_level_nodes_proof, size, &counter);

    if (memcmp(new_root->hash, root->hash, HASH_SIZE) == 0) {
        printf("verified\n");
    } else {
        printf("data has been modified\n");
    }

    free_tree(root);
    return 0;

}