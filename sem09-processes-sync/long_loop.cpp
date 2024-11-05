#include <iostream>
#include <unistd.h>

int main() {
    for (int i = 0; i < 100'000'000; ++i) {
        std::cout << "iteration: " << i << '\n';
        sleep(1);
    }

}
