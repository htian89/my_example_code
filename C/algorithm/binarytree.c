#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 20 
#define SWAP(x, y) {int t; t = x; x = y; y = t;}

void createheap(int *num);
void heapsort(int *num);

int main(void) {
    int number[MAX + 1] = {-1};
    srand(time(NULL));
    printf("Start create number");
    int i;
    for(i = 1; i <= MAX; i++){
        number[i] = rand() % 100;
    }
    createheap(number);
    
    heapsort(number);
    printf("Sort completed\n");
}

void createheap(int *num){
    int heap[MAX + 1] = {-1};
    int i;
    for(i = 1; i <= MAX; i++){
        heap[i] = num[i];
        int s = i;
        int p = s / 2;
        while(s >= 2 && (heap[p] > heap[s])) {
            SWAP(heap[p], heap[s]);
            s = p;
            p = s / 2;
        }
    }

    printf("Created heap:\n");
    for (i = 1; i <= MAX; i++) {
        num[i] = heap[i];
        printf("%d,",num[i]);
    }
    printf("\n");
}

void heapsort(int *num) {
    int m = MAX;
    for(; m > 1;) {
        SWAP(num[1], num[m]);
        m--;
        int p = 1;
        int s = 2 * p;
        while(s <= m) {
            if(s < m && num[s] > num[s + 1])
                s++;
            if(num[s] >= num[p])
                break;

            SWAP(num[s], num[p]);
            p = s;
            s = 2 * p;
        }
    } 

    printf("Sorted heap:\n");
    int i;
    for(i = MAX; i >= 1; i--)
        printf("%d,",num[i]);
    printf("\n");
}
