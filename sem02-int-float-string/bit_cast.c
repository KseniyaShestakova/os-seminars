#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>

void output_int(int64_t a_int) {
    for (size_t i = sizeof(a_int) * CHAR_BIT; i > 0; --i) {
        printf("%ld", (a_int & ((int64_t)1 << (i-1))) >> (i - 1));
     }
     printf("\n");

}

int64_t ptr_cast(double a) {
    double* a_ptr_double = &a;
    int64_t* a_ptr_int = (int64_t*)a_ptr_double;
 
    int64_t a_int = *a_ptr_int;
    return a_int;
}

typedef union {
    double double_value;
    uint64_t uint_value;
} double_or_uint;


int64_t union_cast(double a) {
    double_or_uint u;

    u.double_value = a;

    return u.uint_value;
}



int main() {
    double a = 0.01;

    printf("Bit presentation of %lf, ptr_cast: \n", a);
    output_int(ptr_cast(a));

    printf("Bit presentation of %lf, union_cast: \n", a);
    output_int(union_cast(a));

    return 0;
}
