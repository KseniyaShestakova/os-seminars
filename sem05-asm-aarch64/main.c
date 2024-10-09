#include <stdio.h>
#include <stdint.h>

int64_t get();

int main()  {
    int64_t read = get();

    printf("Got: %ld\n", read);
}
