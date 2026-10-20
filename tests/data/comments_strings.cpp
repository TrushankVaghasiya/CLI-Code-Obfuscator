
#include <iostream>

int main()
{
    // This is a comment.
    const char *first = "text containing // comment markers";
    const char *second = "text containing /* comment markers */";

    std::cout << first << '\n';
    std::cout << second << '\n';
    return 0;
}
