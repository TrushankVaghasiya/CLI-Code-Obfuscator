
#include <iostream>

int main() {
    // This line comment should be removed.
    int number = 5; /* This block comment should be removed. */

    const char* message = "This // is not a comment";
    std::cout << number << '\n';
    std::cout << message << '\n';

    return 0;
}
