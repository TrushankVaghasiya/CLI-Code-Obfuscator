#include <iostream>

int printValue(int value) {
    return value;
}

int main() {
    int number = 7;
    int* pointer = &number;

    std::cout << printValue(*pointer) << '\n';
    return 0;
}