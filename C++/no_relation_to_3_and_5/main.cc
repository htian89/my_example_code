#include <iostream>
using namespace std;

int relate_a(int, int);

int main() {
    int n,i,sum = 0;
    cin >> n;

    for(;i <= n; i++) {
        if (relate_a(i, 3) == 0 && relate_a(i, 5) == 0) {
            sum += i * i;
        }
    }

    cout << sum << endl;
    return 0;
}

int relate_a(int a, int b) {
    if (a % b == 0) {
        return 1;
    }

    while (a != 0) {
        if (a % 10 == b) {
            return 1;
        }
        a = a / 10;
    }
    return 0;
}
