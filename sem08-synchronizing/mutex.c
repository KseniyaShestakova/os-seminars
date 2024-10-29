#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    pthread_mutex_t* mutex;
    double* list;
    size_t sz;
    size_t idx;
    size_t it_cnt;
} worker_t;

void init_worker(
    worker_t* worker,
    pthread_mutex_t* mutex,
    double* list,
    size_t sz,
    size_t idx,
    size_t it_cnt)
{
    worker->mutex = mutex;
    worker->list = list;
    worker->sz = sz;
    worker->idx = idx;
    worker->it_cnt = it_cnt;
}

void* routine(void* arg)
{
    worker_t* wk = (worker_t*)arg;

    size_t left = (wk->idx == 0) ? (wk->sz - 1) : (wk->idx - 1);
    size_t right = (wk->idx == wk->sz - 1) ? 0 : (wk->idx + 1);

    for (int i = 0; i < wk->it_cnt; ++i) {
        pthread_mutex_lock(wk->mutex); // acquire mutex

        wk->list[left] += 0.99;
        wk->list[right] += 1.01;
        wk->list[wk->idx] += 1;

        pthread_mutex_unlock(wk->mutex); // release mutex
    }
    return NULL;
}

int main(int argc, char** argv)
{
    int k = atoi(argv[1]);
    int n = atoi(argv[2]);

    double* list = (double*)calloc(n, sizeof(double));
    pthread_t* thread = (pthread_t*)calloc(n, sizeof(pthread_t));

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    worker_t* worker = (worker_t*)malloc(n * sizeof(worker_t));

    for (int i = 0; i < n; ++i) {
        init_worker(&worker[i], &mutex, list, n, i, k);
    }
    for (int i = 0; i < n; ++i) {
        pthread_create(&thread[i], NULL, &routine, &worker[i]);
    }
    for (int i = 0; i < n; ++i) {
        pthread_join(thread[i], NULL);
    }
    for (int i = 0; i < n; ++i) {
        printf("%.10g ", list[i]);
    }

    pthread_mutex_destroy(&mutex);
    free(worker);
    free(thread);
    free(list);

    return 0;
}
