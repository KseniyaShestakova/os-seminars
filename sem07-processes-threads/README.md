# Процессы и потоки


## Процессы
У каждого процесса существует свои обособленные:
 * адресное пространство начиная с `0x00000000`;
 * набор файловых дескрипторов для открытых файлов.

Кроме того, каждый процесс может находиться в одном из состояний:
 * работает (Running);
 * приостановлен до возникновения определенного события (Suspended);
 * приостановлен до явного сигнала о том, что нужно продолжить работу (sTopped);
 * более не функционирует, не занимает память, но при этом не удален из таблицы процессов (Zombie).

Каждый процесс имеет свой уникальный идентификатор - Process ID (PID), который присваивается системой инкрементально. Множество доступных PID является ограниченным, и его исчерпание проводит к невозможности создания нового процесса.


**Работа с процессами из терминала:**
* Получить полный список процессов: `ps -A`, `ps aux`. 
* Убить процесс: `kill <signal> <pid>`. Обычно вместо `<signal>` пишем `-9`, т.е. посылаем сигнал `SIGKILL`




```bash
%%bash
ps aux | head -n 5
```

    USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
    root           1  0.0  0.1 164920 11144 ?        Ss   11:37   0:01 /sbin/init splash
    root           2  0.0  0.0      0     0 ?        S    11:37   0:00 [kthreadd]
    root           3  0.0  0.0      0     0 ?        I<   11:37   0:00 [rcu_gp]
    root           4  0.0  0.0      0     0 ?        I<   11:37   0:00 [rcu_par_gp]



```bash
%%bash
ps aux | tail -n 5
```

    root        6251  0.0  0.0      0     0 ?        I    12:16   0:00 [kworker/5:1-events]
    root        6272  0.0  0.0      0     0 ?        I    12:17   0:00 [kworker/u16:2-ext4-rsv-conversion]
    xxeniash    6273  0.0  0.0   9940  3344 ?        S    12:17   0:00 bash
    xxeniash    6275  0.0  0.0  12792  3772 ?        R    12:17   0:00 ps aux
    xxeniash    6276  0.0  0.0   8400   980 ?        S    12:17   0:00 tail -n 5


Процессы иерархичны. Существует процесс `init` с `PID=1`, он порождает дочерние процессы, которые в свою очередь тоже порождают какие-то процессы и т.д.

Процессы объединяются в `process group`, которым доставляются сигналы о наступленнии некоторых событий. Пример: процессы, запущенные из одного терминала (призакрытии терминала они будут убиты).

Объединение нескольких групп процессов называется *сеансом* (`session`). Как правило, в один сеанс объединяются процессы в рамках одного входа пользователя в систему.

### Системный вызов Fork

Предназначен для создания новых процессов.

```c
#include <unistd.h>

pid_t fork();

```

По возвращаемому значению можно отличить родительский и дочерний процессы:

```cpp
pid_t process_id; // в большинстве систем pid_t совпадает с int
if ( -1 == ( process_id=fork() ) ) {
  perror("fork"); // ошибка создания нового процесса
}
else if ( 0 == process_id ) {
  printf("I'm a child process!");
}
else {
  printf("I'm a parent process, created child %d", process_id);
}
```

Может быть использован для создания fork-бомбы - программы, которая занимается тем, что исчерпывает лимит запущенных процессов:

```cpp
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

int main()
{
    char * are_you_sure = getenv("ALLOW_FORK_BOMB");
    if (!are_you_sure || 0!=strcmp(are_you_sure, "yes")) {
        fprintf(stderr, "Fork bomb not allowed!\n");
        exit(127);
    }

    pid_t pid;
    do {
        pid = fork();
    } while (-1 != pid);

    printf("Process %d reached out limit on processes\n", getpid());
    while (1) {
        sched_yield();
    }
}
```

Для того, чтобы безопасно протестировать fork-бомбу, рекомендуется создать нового пользователя и установить ему лимит на число запущенных потоков/процессов:

```bash
sudo useradd tmp_user # создаем пользователя
sudo passwd tmp_user  # устанавливаем пароль
su tmp_user           # логинимся под пользователя в этом окне терминала
ulimit -u 100         # ограничиваем число потоков/процессов доступное пользователю
./inf09_0.exe         # запускаем опасную программу
```

### Чтение кода возврата дочернего процесса
 * `wait(int *wstatus)` - ожидание завершения любого дочернего процесса, возвращает информацию о завершении работы;
 * `waitpid(pid_t pid, int *wstatus, int options)` - ожидание (возможно неблокирующее) завершения работы конкретного процесса, возвращает информации о завершении работы;
 * `wait3(int *wstatus, int options, struct rusage *rusage)` - ожидание (возможно неблокирующее) завершения любого дочернего процесса, возвращает информацию о завершении работы и статистике использования ресурсов;
 * `wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage)` - ожидание (возможно неблокирующее) завершения конкретного процесса, возвращает информацию о завершении работы и статистике использования ресурсов.

### Системный вызов exec

Предназначен для замены программы текущего процесса. Чаще всего используется с `fork` для того, чтобы запустить из текущей программы некоторую стороннюю (например, скрипт на python или bash).

```cpp
int execve(const char *filename,
           char *const argv[],
           char *const envp[]);           
int execvpe(.....) // параметры аналогично execve

int execv(const char *filename, char *const argv[])
int execvp(......) // параметры аналогично execv

int execle(const char *filename,
           const char arg0, ..., /* NULL */,
           const char env0, ..., /* NULL */);

int execl(const char *filename,
          const char arg0, ..., /* NULL */);
int execlp(......) // параметры аналогично execl

```

Различные буквы в суффиксах названий `exec` означают?
 * `v` или `l` - параметры передаются в виде массивов (`v`), заканчивающихся элементом `NULL`, либо в виде переменного количества аргументов (`l`), где признаком конца перечисления аргументов является значение `NULL`.
 * `e` - кроме аргументов программы передаются переменные окружения в виде строк `КЛЮЧ=ЗНАЧЕНИЕ`.
 * `p` - именем программы может быть не только имя файла, но и имя, которое нужно найти в одном из каталогов, перечисленных в переменной окружения `PATH`.

Возвращаемым значением `exec` может быть только значение `-1` (признак ошибки). В случае успеха, возвращаемое значение уже в принципе не имеет никакого смысла, поскольку будет выполняться другая программа.

Аргументы программы - это то, что передаётся в функцию `main` (на самом деле, они доступны из `_start`, поскольку располагаются на стеке). Первым аргументом (с индексом `0`), как правило, является имя программы, но это не является обязательным требованием.


Классическое использование:
```cpp
if (0 == fork()) { // создали дочерний процесс, из него запускам program
  execlp(program, program, NULL);
  perror("exec"); exit(1);
}
```


```python
%%file fork_exec.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
    pid_t pid;
    if ((pid = fork()) == 0) {
        // execlp("ps", "ps", "aux", NULL); // also possible variant
        execlp("echo", "echo", "Hello world from linux ECHO program", NULL);
        // execlp("sleep", "sleep", "3", NULL);
        // execlp("bash", "bash", "-c", "ps aux | head -n 4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    int status;
    struct rusage resource_usage;
    pid_t w = wait4(pid, &status, 0, &resource_usage); // обязательно нужно дождаться, пока завершится дочерний процесс
    if (w == -1) {
        perror("waitpid");
        exit(-1);
    }
    assert(WIFEXITED(status));
    printf("Child exited with code %d \n"
           "\tUser time %ld sec %ld usec\n"
           "\tSys time %ld sec %ld usec\n", 
           WEXITSTATUS(status), 
           resource_usage.ru_utime.tv_sec,
           resource_usage.ru_utime.tv_usec,
           resource_usage.ru_stime.tv_sec,
           resource_usage.ru_stime.tv_usec); // выводим код возврата дочернего процесса + еще полезную информацию
    
    return 0;
}
```

    Overwriting fork_exec.c



```bash
%%bash 
gcc fork_exec.c -o fork_exec
./fork_exec
```

    Hello world from linux ECHO program
    Child exited with code 0 
    	User time 0 sec 368 usec
    	Sys time 0 sec 0 usec


## Потоки

Поток - единица планирования времени в рамках одного процесса. Чтобы понять, чем процессы отличаются от потоков, посмотрим на их атрибуты.

**Атрибуты процесса:**
* виртуальное адресное пространство и данные в нем
* файловые дескрипторы и блокировки файлов
* PID
* argc, argv
* ulimit

**Атрибуты потоков:**
* маски сигналов
* состояние процесса R, S, T, Z
* состояние регистров (какая функция сейчас выполняется)
* TID

Все потоки в рамках одного процесса разделяют общее адресное пространство и открытые файловые дескрипторы.

Для каждого потока предусмотрен свой отдельный стек фиксированного размера, который располагается в общем адресном пространстве.

Есть функции, с помощью которых можно задавать опеределенные параметры создаваемого потока:
 * `pthread_attr_setstacksize` - установить размер стека для потока. Размер стека должен быть кратен размеру страницы памяти (обычно 4096 байт), и для него определен минимальный размер, определяемый из параметров системы `sysconf(_SC_THREAD_STACK_MIN)` или константой `PTHREAD_STACK_MIN` из `<limits.h>` (в Linux это 16384 байт);
 * `pthread_attr_setstackaddr` - указать явным образом адрес размещения памяти, которая будет использована для стека;
 * `pthread_attr_setguardsize` - установить размер защитной области после стека (Guard Page). По умолчанию в Linux этот размер равен размеру страницы памяти, но можно явно указать значение 0.
 
### Функции для работы с потоками

Создание и запуск нового потока:
```cpp
#include <pthread.h>

int pthread_create(/* указатель на результат */
                   pthread_t *restrict thread, 
                   /* атрибуты (может быть NULL) */
                  const pthread_attr_t *attr,
                   /* функция, которую будет выполнять поток */
                  void *(*start_routine)(void *),
                  /* аргументы функции */
                  void *restrict arg);
```


Ключевое слово `restrict` означает, что указатель, перед которым оно указано, - единственный указатель, по которому будет изменяться соответствующий участок памяти.

Поток завершается в тот момент, в который завершается выполнение функции, либо пока не будет вызван аналог `exit` для потока - функция `pthread_exit`.

Возвращаемые значения размером больше одного машинного слова, которые являются результатом работы потока, не могут быть размещены в стеке, поскольку стек будет уничтожен при завершении работы функции.

```cpp
int pthread_join(
    // поток, который нужно ждать
    pthread_t thread,

    // указатель на результат работы функции,
    // либо NULL, если он не интересен
    (void*) *retval
    );
```
Функция `pthread_join` ожидает завершения работы определенного потока, и получает результат работы функции.


Если два потока вызовут ожидание друг друга, то произойдет deadlock (и программа зависнет).

Функция `pthread_cancel` принудительно завершает работу потока, если поток явно это не запретил с помощью функции `pthread_setcancelstate`.

```cpp
int pthread_cancel(
    // поток, который нужно прибить
    pthread_t thread
    );
```

Результатом работы функции, который будет передан в `pthread_join` будет специальное значение `PTHREAD_CANCELED`.


```python
%%file recursive_read.c

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
 
void* start_routine(void* arg)
{
     int32_t curr;
     if (scanf("%d", &curr) == EOF) {
         return NULL;
     }
     pthread_t next;
     pthread_create(&next, NULL, &start_routine, NULL);
     pthread_join(next, NULL);
     printf("%d ", curr);
     return NULL;
}

int main()
{
     pthread_t init;
     pthread_create(&init, NULL, &start_routine, NULL);
     pthread_join(init, NULL);
     return 0;
}
```

    Writing recursive_read.c



```bash
%%bash
gcc recursive_read.c -o recursive_read
cat input.txt | ./recursive_read
```

    3 2 2 1 

## Работа с исполняемыми файлами в Linux

**ELF (Executable and Linkable Format)** - стандартный формат для исполняемых файлов в Unix-системах.

Секции ELF-файла:
* ELF header
* Program header table (описывает executable/writeable/readable сегменты)
* Section header table
* Данные, на которые ссылаются сущности из program header table и section header table, например, `.text`, `.rodata`, `.data`

Попробуем поразбираться с файлом `recursive_read`, полученным на предыдущем шаге. Сначала проверим, что он имеет формат ELF, прочитав его заголовок с помощью `objdump`.


```bash
%%bash
objdump -f recursive_read
```

    
    recursive_read:     file format elf64-x86-64
    architecture: i386:x86-64, flags 0x00000150:
    HAS_SYMS, DYNAMIC, D_PAGED
    start address 0x00000000000010e0
    


Для чтения ELF-файлов можно использовать утилиты `objdump` и `readelf`. Подробнее про то, какие они принимают опции, можно узнать в `man`-е. Сейчас разберем несколько примеров.

* `readelf -a` - вся информация (`--all`)
* `readelf -l` - информация о program headers (`--program-headers | --segments`)
* `readelf -S` - информация о section headers (`--section headers } --sections`)
* `readelf -s` - информация о символах (в частности, в случае с recursive_read увидим информацию о `strat_routine`, `pthread_join`, `_init`)


```python

```
