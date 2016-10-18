//Find the max product of consecutive elements in an array.

#include <stdio.h>
#include <stdlib.h>

#define MAX(x, y) ((x > y) ? x : y)
#define MIN(x, y) ((x > y) ? y : x)

int find_max_product(int a[], int a_len){
    int i = 0, j = 0;
    int max = a[0];
    for(; i < a_len; i++){
        int product = 1;
        for(j = i; j < a_len; j++){
            product *= a[j];
            max = MAX(max, product);
        }
    }
    return max;
}

int find_max_product_enhanced(int a[], int a_len){
    int res = a[0];
    int max = a[0];
    int min = a[0];
    int max_end = a[0];
    int min_end = a[0];
    int i = 1;
    for(; i < a_len; i++){
        max_end = max * a[i];
        min_end = min * a[i];
        max = MAX(MAX(max_end, min_end), a[i]);
        min = MIN(MIN(max_end, min_end), a[i]);
        res = MAX(res, max);
    }
    return res;
}

int main(void) {
    int max = 0;
    int array[10] = {0};
    int array_len = 0;
    int data;
    while(scanf("%d", &data) != EOF) {
        array[array_len] = data;
        array_len++;
    }
    max = find_max_product(array, array_len);
    printf("Max product is: %d\n", max);
    max = find_max_product_enhanced(array, array_len);
    printf("Max product is: %d\n", max);
    return 0;
}
