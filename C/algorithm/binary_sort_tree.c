//Binary sort tree

#include <stdio.h>
#include <stdlib.h>

typedef struct _tree{
    int data;
    struct _tree *left;
    struct _tree *right;
} tree;

tree *insert(tree *node, int data){
    if(node == NULL){
        node = malloc(sizeof(tree));
        node->data = data;
        node->left = NULL;
        node->right = NULL;
        return node;
    }

    if(data < node->data){
        node->left = insert(node->left, data);
    } else {
        node->right = insert(node->right, data);
    }
    
    return node;
}

void dump_tree_ascending(tree *node){
    if(node->left != NULL) {
        dump_tree_ascending(node->left);
    }

    printf("%d ", node->data);

    if(node->right){
        dump_tree_ascending(node->right);
    }
}

int main(void){
    tree *t = NULL;
    int data;
    while(scanf("%d", &data) != EOF){
        t = insert(t, data);
    }
    dump_tree_ascending(t);
    printf("\n");
    return 0;
}
