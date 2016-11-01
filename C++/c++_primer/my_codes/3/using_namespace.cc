#include <iostream>

using std::cin;
using std::cout; using std::string;

int main() {
    string in("Initial value");
    getline(cin, in);
    cout << "Input string : " << in << std::endl; 
    cout << (in.size() < -1) << std::endl;
    return 0;
}
