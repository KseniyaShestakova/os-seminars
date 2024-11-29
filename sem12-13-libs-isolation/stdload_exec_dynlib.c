
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <dlfcn.h>

typedef float (*binary_float_function)(float, float);

int main() {  
    
    void *lib_handle = dlopen("./libsum.so", RTLD_NOW);
    if (!lib_handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        abort();
    }
   
    int (*sum)(int, int) = dlsym(lib_handle, "sum_int");
    binary_float_function sum_f = dlsym(lib_handle, "sum_float");
    
    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);
    p(sum(1, 1), "%d");
    p(sum(40, 5000), "%d");
    
    p(sum_f(1, 1), "%0.2f");
    p(sum_f(4.0, 500.1), "%0.2f");
    
    dlclose(lib_handle);
    return 0;
}
