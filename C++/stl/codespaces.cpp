#include <iostream>
#include <unordered_map>

int main(){
    std::unordered_map<int32_t, std::string> id2string;
    id2string[0] = "a";
    id2string[1] = "b";
    std::cout << "contains="  << " value=" << id2string[0] << std::endl;

    // bitmap test
    int32_t bitmap_tmp = 1234;
    for (int32_t i = 0, bitmap = bitmap_tmp; bitmap > 0;  bitmap>>=1, ++i) {
        if ((bitmap & 1) == 0) continue;
        std::cout << "index=" << i << " bitmap=" << bitmap << std::endl;
    }

    return 0;
}
