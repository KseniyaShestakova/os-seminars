#include <iostream>
#include <vector>


int foo(int a, int b) {
    return a + b;
}

int main() {
    int c = foo(5, 10);
    std::cout << c << std::endl;
}
