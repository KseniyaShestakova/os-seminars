#include <pthread.h>
#include <inttypes.h>
#include <stdio.h>

int64_t balance;
int N = 100000;

// mutex

void* thread_func() {
    for (int i = 0; i < N; ++i) {
        // mutex.lock
        balance += 1;
        // mutex.unlock
    }
}

int main() {
    pthread_t one;
    pthread_t two;

    pthread_create(&one, NULL, &thread_func, NULL);
    pthread_create(&two, NULL, &thread_func, NULL);

    pthread_join(one, NULL);
    pthread_join(two, NULL);

    printf("%ld\n", balance);

}


