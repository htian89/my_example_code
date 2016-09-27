#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void reverse(char in[], char out[], int len) {
    int need_reverse_len = 0;
    int start_reverse_flag = 0;
    int i;
    for (i = 0; i <= len; i++) {
        if ((in[i] >= 'a' && in[i] <= 'z') || (in[i] >= 'A' && in[i] <= 'Z')) {
            need_reverse_len++;
        } else {
            start_reverse_flag = (need_reverse_len > 0) ? 1 : 0;
            out[i] = in[i];
        }

        if (start_reverse_flag){
            int reversed_len = 0;
            for (; reversed_len < need_reverse_len; reversed_len++) {
                out[i - need_reverse_len + reversed_len] 
                    = in[i - 1 - reversed_len];
            }
            need_reverse_len = 0;
            start_reverse_flag = 0;
        }
    }
}

int main(void) {
    char in[256];
    char out[256];
    int first_word_flag = 1;
    while (scanf("%s", in) != EOF) {
        if (first_word_flag) 
            first_word_flag = 0;
        else
            printf(" ");
        int len = strlen(in);    
        reverse(in, out, len);
        printf("%s", out);
    }
    printf("\n");
}
