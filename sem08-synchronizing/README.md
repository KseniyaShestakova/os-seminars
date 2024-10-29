# Синхронизация потоков

*Атомарными* называются операции без промежуточных стадий, т.е. те, которые могут быть либо еще не начаты, либо уже завршены.

На всех современных процессорных архитектурах операции чтения и записи в память выровненных данных, размер которых не превышает машинного слова, являются атомарными. Однако, с точки зрения строгому следованию стандартам Си и C++, такое предположение неверно, и необходимо использовать специальные типы данных и атомарные операции. Кроме того, операции над типами данных, размер которых превышает размер машинного слова, заведомо являются неатомарными.

*Гонка данных (data race)* - ситуация, когда несколько потоков или процессов обращаются к одной области памяти, причем хотя бы один из них проводит запись. Гонка данных приводит к неопределенному поведению.

Рассмотрим классический пример с гонкой данных.


```python
%cat data_race.c
```

    #include <pthread.h>
    #include <inttypes.h>
    #include <stdio.h>
    
    int64_t balance;
    int N = 100000;
    
    void* thread_func() {
        for (int i = 0; i < N; ++i) {
            balance += 1;
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
    
    



```bash
%%bash
gcc -O0 data_race.c -o data_race
./data_race
./data_race
./data_race
./data_race
```

    155243
    143527
    156902
    169742


Видим, что ответ не стабильный и получается меньше ожидаемого 200000. Отловить неопределенное поведение можно было бы  помощью Thread санитайзера:

```
gcc -fsanitize=thread -O0 data_race.c -o data_race
./data_race
```

## Критические секции и Mutex

Часть программы, которая подразумевает монопольное использование какого-либо набора переменных, называется *критической секцией*.

*Мьютекс* - примитив синхронизации который обеспечивает монопольный доступ к критической секции.

Мьютекс объявлен в заголовочном файле `<pthread.h>`, и функции работы с ним требуют линковки с библиотекой `-pthread`.

 * `pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)` - инициализация мьютекса для его последующего использования. Если второй параметр `NULL`, то инициализируется обычный (не рекурсивный) мьютекс. Для создания нового инициализированного мьютекса с параметрами по умолчанию можно использовать макрос `PTHREAD_MUTEX_INITIALIZER`.
 * `pthread_mutex_destroy(pthread_mutex_t *mutex)` - уничтожить ранее созданный мьютекс.
 * `pthread_mutex_lock(pthread_mutex_t *mutex)` - захватить мьютекс. Если другой поток уже захватил его, то текущий поток приостанавливает свою работу.
 * `pthread_mutex_trylock(pthread_mutex_t *mutex)` - пытается захватить мьютекс. В случае успеха возвращает значение `0`, а если мьютекс уже занят, то значение `EBUSY`.
 * `pthread_mutex_unlock(pthread_mutex_t *mutex)` - освободить ранее захваченный мьютекс. В отличии от семафоров, освободить мьютекс может только тот поток, которые его захватил, в противном случае это приведет к ошибке `EPERM`.


```python
%cat mutex.c
```

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



```bash
%%bash
gcc -fsanitize=thread mutex.c -o mutex
./mutex 10 10
```

    30 30 30 30 30 30 30 30 30 30 

## Условные переменные

*Условная переменная* - примитив синхронизации, нужный для оповещения одним из потоков остальных о наступлении некоторого события, например, о готовности данных.

  * `pthread_cond_init(pthread_cond_t *c, const pthread_condattr_t *attr)` - инициализации условной переменной. Второй параметр может быть `NULL` - в этом случае подразумевается использование переменной только в рамках одного процесса. Для инициализации условной переменной с параметрами по умолчанию используется макрос `PTHREAD_COND_INITIALIZER`.
  * `pthread_cond_destroy(pthread_cond_t *c)` - уничтожить условную переменную.
  * `pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)` - ожидает нотификации условной переменной переменной `c`, временно разблокируя мютекс `m`. Перед вызовом мьютекс должен находиться в заблокированном состоянии, в противном случае - неопределенное поведение. После наступления события нотификации, мьютекс опять блокируется.
  * `pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *timeout)` - то же, что и `pthread_cond_wait`, но ожидание прекращается по истечению указанного периода времени.
  * `pthread_cond_signal(pthread_cond_t *c)` - уведомляет один поток, для которого выполняется ожидание нотификации. В общем случае, поток выбирается случайным образом, если их несколько.
  * `pthread_cond_broadcast(pthread_cond_t *c)` - уведомляет все потоки, для которых выполняется ожидание нотификации.

В случае, если ни один поток не вызвал `pthread_cond_wait`, то нотификация условной переменной проходит незамеченной.



```python
%%file condvar.c

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef struct {
    pthread_mutex_t mutex; // мьютекс
    pthread_cond_t condvar; // переменная условия (если нужна)
    
    int value;
} promise_t;

void promise_init(promise_t* promise) {
    pthread_mutex_init(&promise->mutex, NULL);
    pthread_cond_init(&promise->condvar, NULL);
    promise->value = -1;
}

void promise_set(promise_t* promise, int value) {
    pthread_mutex_lock(&promise->mutex); // try comment lock&unlock out and look at result
    promise->value = value; // криитическую секцию стоит делать как можно меньше
    pthread_mutex_unlock(&promise->mutex);
    pthread_cond_signal(&promise->condvar); // notify if there was nothing and now will be elements
}

int promise_get(promise_t* promise) {
    pthread_mutex_lock(&promise->mutex); // try comment lock&unlock out and look at result
    while (promise->value == -1) {
        pthread_cond_wait(&promise->condvar, &promise->mutex);
    }
    int value = promise->value;
    pthread_mutex_unlock(&promise->mutex);
    return value;
}

promise_t promise_1, promise_2;


static void* thread_A_func(void* arg) {
    log_printf("Func A started\n");
    promise_set(&promise_1, 42);
    log_printf("Func A set promise_1 with 42\n");
    int value_2 = promise_get(&promise_2);
    log_printf("Func A get promise_2 value = %d\n", value_2);
    return NULL;
}

static void* thread_B_func(void* arg) {
    log_printf("Func B started\n");
    int value_1 = promise_get(&promise_1);
    log_printf("Func B get promise_1 value = %d\n", value_1);
    promise_set(&promise_2, value_1 * 100);
    log_printf("Func B set promise_2 with %d\n", value_1 * 100)
    return NULL;
}

int main()
{
    promise_init(&promise_1);
    promise_init(&promise_2);
    
    log_printf("Main func started\n");
    
    pthread_t thread_A_id;
    log_printf("Creating thread A\n");
    pt_verify(pthread_create(&thread_A_id, NULL, thread_A_func, NULL));
    
    pthread_t thread_B_id;
    log_printf("Creating thread B\n");
    pt_verify(pthread_create(&thread_B_id, NULL, thread_B_func, NULL));
    
    pt_verify(pthread_join(thread_A_id, NULL)); 
    log_printf("Thread A joined\n");
    
    pt_verify(pthread_join(thread_B_id, NULL)); 
    log_printf("Thread B joined\n");
    
    log_printf("Main func finished\n");
    return 0;
}

```

    Writing condvar.c



```bash
%%bash
gcc -fsanitize=thread condvar.c -lpthread -o condvar
./condvar
```

    0.000          main():96  [tid=14380]: Main func started
    0.000          main():99  [tid=14380]: Creating thread A
    0.001          main():103 [tid=14380]: Creating thread B
    0.001 thread_A_func():74  [tid=14382]: Func A started
    0.001 thread_A_func():76  [tid=14382]: Func A set promise_1 with 42
    0.001 thread_B_func():83  [tid=14383]: Func B started
    0.001 thread_B_func():85  [tid=14383]: Func B get promise_1 value = 42
    0.001 thread_B_func():87  [tid=14383]: Func B set promise_2 with 4200
    0.001 thread_A_func():78  [tid=14382]: Func A get promise_2 value = 4200
    0.001          main():107 [tid=14380]: Thread A joined
    0.001          main():110 [tid=14380]: Thread B joined
    0.001          main():112 [tid=14380]: Main func finished


## Атомарные переменные

Атомарные переменные - те, операции над которыми атомарны :)

Для того, чтобы объявить атомарную переменную, используется ключевое слово `_Atomic`. В C  атомарными могут быть только те типы, которые помещаются в машинное слово, в то время как в C++ атомарным может быть любой объект (для достижения атомарности составных типов там используется мьютекс).

Атомарные операции над типами, объявленными как `_Atomic`, реализуются в Си11 как макросы:

 * `void atomic_store(T* object, T value)`,
 * `void atomic_store_explicit(T* object, T value, memory_order order)` - сохранить значение в атомарную переменную.
 * `T atomic_load(T* object)`,
 * `T atomic_load_explicit(T* object, memory_order order)` - загрузить значение из переменной.
 * `T atomic_exchange(T* object, T new_value)`,
 * `T atomic_exchange_explicit(T* object, T new_value, memory_order order)` - заменить значение и вернуть предыдущее.
 * `_Bool atomic_compare_exchange_strong(T* object, T* expected, T new_value)`,
 * `_Bool atomic_compare_exchange_strong_explicit(T* object, T* expected, T new_value, memory_order success, memory_order failure)`,
 * `_Bool atomic_compare_exchange_weak(T* object, T* expected, T new_value)`,
 * `_Bool atomic_compare_exchange_weak_explicit(T* object, T* expected, T new_value, memory_order success, memory_order failure)` - сравнить два значения, в случае их равенства - заменить на новое, в противном случае - записать в `expected` значение `object`.
 * `T atomic_fetch_MOD(T* object, T operand)`,
 * `T atomic_fetch_MOD_explicit(T* object, T operand, memory_order order)` - получить значение переменной, после чего - модифицировать её. `MOD` можеть быть:
  - `add` - инкремент
  - `sub` - декремент
  - `and` - поразрядное "и"
  - `or` - поразрядное "или"
  - `xor` - поразрядное "исключающее или".
  
  
## Неблокирующие структуры данных

Использование мьютексов и условных переменных часто приводит к простаиванию потоков, в частности, когда поток собирается прочитать переменную, охраняемую мьютексом, но не собирается ее модифицировать. Для того, чтобы избежать таких ситуаций, часто используются lockfree структуры данных, которые, как правило, реализуются на основе атомарных операций.

В домашнем задании вам предстоит написать свою реализацию lockfree stack-а. Сейчас разберем, как можно с помощью атомарных операций делать push в lockfree stack.


```python
%%file lockfree_stack.c

typedef struct node_t {
    struct node_t* next;
    int value;
} node_t;

typedef struct lockfree_stack_t {
    _Atomic(node_t*) top;
} lockfree_stack_t;

int lockfree_stack_init(lockfree_stack_t* stack) {
    if (stack == NULL) {
        return 1;
    }
    
    atomic_init(&stack->top, NULL);
    return 0;
}

int lockfree_stack_push(lockfree_stack_t* stack, int value) {
    if (stack == NULL) {
        return 1;
    }

    node_t *new_node = (node_t*) calloc(1, sizeof(node_t));
    new_node->value = value;

    node_t *old_top = NULL;

    while (1) {
        node_t *old_top = stack->top;
        new_node->next = old_top;

        if (atomic_compare_exchange_weak(&stack->top, &old_top, new_node))
            break;
    }

    return 0;
}
```

    Writing lockfree_stack.c


## Больше практики

Советую посмотреть [ноутбук](https://github.com/yuri-pechatnov/caos/blob/master/caos_2020-2021/sem21-synchronizing/quiz.md) от Юрия Печатнова с задачами на синхронизацию потоков и процессов.


```python

```
