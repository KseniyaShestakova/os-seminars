#include <stdio.h>

int access(int* a, int i) { 
    return a[i]; 
}

int main() {
    int a[2] = {41, 42};
    int i = 100501;
    printf("a[%d] = %d\n", i, access(a, i)); // обращение к "чужой" памяти
}
