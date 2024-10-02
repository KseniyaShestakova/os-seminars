#include <inttypes.h>
#include <stdio.h>

extern int *A;

void cyclic_sum(size_t N) {
    for (size_t i = 0; i < N; ++i) {
        int32_t num = 0;
        scanf("%d", &num);
        int64_t result = num;
        result *= A[i];
        printf("%d\n", result);
    }    
}
