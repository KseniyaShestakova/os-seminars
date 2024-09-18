#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool check_inc(int32_t x) {
    return x < x + 1;
}

bool check_uinc(uint32_t x) {
    return x < x + 1;
}


int main() {
    int32_t signed_int = INT32_MAX;
    printf("Signed check for %d: %d\n", signed_int, check_inc(signed_int));
    printf("Increment for %d: %d\n", signed_int, signed_int + 1);

    uint32_t unsigned_int = UINT32_MAX;
    printf("Unsigned check for %u: %u\n", unsigned_int, check_uinc(unsigned_int));
    printf("Increment for %u: %u\n", unsigned_int, unsigned_int + 1);
    
}
