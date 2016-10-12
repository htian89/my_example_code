#include <stdlib.h>
#include <stdio.h>

typedef struct _list {
    int data;
    struct _list *next;
} list;

void dump_list(list *p) {
    printf("data of list: ");
    while(p != NULL) {
        printf(" %02d", p->data);
        p = p->next;
    }
    printf("\n");
}

void reverse(list **p) {
    list *pre = NULL;
    list *next = NULL;
    while(*p != NULL) {
        next = (*p)->next;
        (*p)->next = pre;
        if(next == NULL)
            break;
        pre = *p;
        *p = next;
    }    
}

int main(void) {
    list *header = NULL; 
    list *p = NULL;
    int data;
    while(scanf("%d", &data) != EOF) {
        if(p == NULL) {
            p = malloc(sizeof(list));
            header = p;
        } else {
            p->next = malloc(sizeof(list));
            p = p->next;
        }
        p->data = data;
        p->next = NULL;
    } 
    dump_list(header);
    reverse(&header);
    dump_list(header);
}
