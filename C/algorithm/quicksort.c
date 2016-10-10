#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 10
#define SWAP(x, y) {int t; t = y; y = x; x = t;}

void quicksort1(int number[], int left, int right);
void quicksort(int *number, int left, int right);

int main(void){
    int number[MAX] = {-1};
    printf("Create random numbers.\n");
    srand(time(NULL));
    int i;
    for(i = 0; i < MAX; i++) {
        number[i] = rand() % 100;
    }
    printf("Unsorted numbers:\n");
    for(i = 0; i < MAX; i++) {
        printf("%04d,", number[i]);
    }
    printf("\n");

    quicksort1(number, 0, MAX - 1);

    printf("Sorted numbers:\n");
    for(i = 0; i < MAX; i++) {
        printf("%04d,", number[i]);
    }
    printf("\n");
}

void quicksort1(int number[], int left, int right) {
    int s = number[left]; 
    int l = left + 1;
    int r = right;
    if(left < right) {
        while(1) {
            while(l <= right && number[l] < s)
                l++;
            while(r >= left + 1 && number[r] > s)
                r--; 
            if (l >= r)
                break;
            SWAP(number[l], number[r]);
            l++;
            r--;
        }
        number[left] = number[r];
        number[r] = s;

        quicksort1(number, left, r - 1);
        quicksort1(number, r + 1, right);
    }
}

void quicksort(int number[], int left, int right) {
    if (left < right) {
        int s = number[(left + right) / 2];
        int l = left;
        int r = right;
        while(1) {
            while(l <= right && number[l] < s)
                l++;
            while(r >= left && number[r] > s)
                r--;
            if(l >= r)
                break;

            SWAP(number[l], number[r]);
            l++; 
            r--;

        }
        quicksort(number, left, l - 1);
        quicksort(number, r + 1, right);
    }
}
