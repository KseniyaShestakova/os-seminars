
#include <stdio.h>

int strange_function_L4(int a, int b, int c);
__asm__ (R"(
.global strange_function_L4
strange_function_L4:
    endbr64
    add    edi, esi
    add    edi, edx
    movsx    rax, edi
    ret
)");

int main() {
    printf("%d\n", strange_function_L4(4, 0, 1));
    printf("%d\n", strange_function_L4(4, 2, 2));
    printf("%d\n", strange_function_L4(5, 0, 3));
    printf("%d\n", strange_function_L4(5, 2, 4));
    return 0;
}
