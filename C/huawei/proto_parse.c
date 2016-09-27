#include <stdio.h>
#include <stdlib.h>

#define TWO_BYTES(x) (data[x] << 8 | data[x + 1])
#define FOUR_BYTES(x) (data[x] << 24 | data[x + 1] << 16 | data[x + 2] << 8 | data[x + 3])

void parse(int data[]) {
    printf("Type=%d\n",data[0]);
    printf("Command=%d\n",data[1]);
    printf("Port=%d\n",TWO_BYTES(2));
    printf("Result=%d\n",TWO_BYTES(4));
    printf("TransMode=%d\n",TWO_BYTES(6));
    printf("RateUp=%d\n",FOUR_BYTES(8));
    printf("RateDown=%d\n",FOUR_BYTES(12));
}

int main(void) {
    int data[20] = {0};
    int i = 0;
    while (scanf("%x", &data[i]) != EOF) {
        printf("%d %d\n", i, data[i]);
        if (i > 15)
            return -1; // Too much data
        if (data[i] > 0xff)
            return -2; // 1 byte can't be greater than 0xff
        i++;
    }
    if (i == 16) {
        parse(data); 
    } else {
        // Too little data
    }
}
