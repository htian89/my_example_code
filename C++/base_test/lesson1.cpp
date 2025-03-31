#include <iostream>
#include <unordered_map>

int main(){
    std::unordered_map<int32_t, std::string> id2string;
    id2string[0] = "a";
    id2string[1] = "b";
    std::cout << "contains=" << id2string.contains(0) << " value=" << id2string[0] << std::endl;

    return 0;
}
