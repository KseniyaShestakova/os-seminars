# Межпроцессное взаимодействие

В прошлый раз мы рассмотрели способы взаимодействия между разными потоками в пределах одного процесса. Сейчас будем разбираться с тем, как разным процессам взаимодействовать друг с другом.

## Пайпы и перенаправление файловых дискрипторов

*Канал (pipe)* - это пара связанных между собой файловых дескрипторов, один из которых предназначен только для чтения, а другой - только для записи. При это данные, записанные во второй из них, можно прочитать из первого.

Канал создается с помощью системного вызова `pipe`:
```
#include <unistd.h>

int pipe(int pipefd[2]);
```

В качестве аргумента системному вызову `pipe` передается указатель на массив и двух целых чисел, куда будут записаны номера файловых дескрипторов:
 * `pipefd[0]` - файловый дескриптор, предназначенный для чтения;
 * `pipefd[1]` - файловый дескриптор, предназначенный для записи.
 
### Запись данных в канал

Осуществляется с помощью системного вызова `write`, первым аргументом которого является `pipefd[1]`. Канал является буферизованным, под Linux обычно его размер 65К. Возможные сценарии поведения при записи:

 * системный вызов `write` завершается немедленно, если размер данных меньше размера буфера, и в буфере есть место;
 * системный вызов `write` приостанавливает выполнение до тех пор, пока не появится место в буфере, то есть предыдущие данные не будут кем-то прочитаны из канала;
 * системный вызов `write` завершается с ошибкой `Broken pipe` (доставляется через сигнал `SIGPIPE`), если с противоположной стороны канал был закрыт, и данные читать некому.

### Чтение данных из канала

Осуществляется с помощью системного вызова `read`, первым аргументом которого является `pipefd[0]`. Возможные сценарии поведения при чтении:

 * если в буфере канала есть данные, то `read` читает их, и завершает свою работу;
 * если буфер пустой и есть **хотя бы один** открытый файловый дескриптор с противоположной стороны, то выполнение `read` блокируется;
 * если буфер пустой и все файловые дескрипторы с противоположной стороны каналы закрыты, то `read` немедленно завершает работу, возвращая `0`.
 
### Пример
Уже знакомое нам `|` в терминале - это канал (pipe). Например, мы испозовали его для перенаправления вывода одной команды на вход другой:


```bash
%%bash
ps aux | tail -n 5
```

    root        9168  0.0  0.0      0     0 ?        I    11:35   0:00 [kworker/5:2-events]
    root        9172  0.0  0.0      0     0 ?        I    11:35   0:00 [kworker/0:1-events]
    xxeniash    9175  0.0  0.0   9940  3480 ?        S    11:35   0:00 bash
    xxeniash    9177  0.0  0.0  12792  3816 ?        R    11:35   0:00 ps aux
    xxeniash    9178  0.0  0.0   8400   972 ?        S    11:35   0:00 tail -n 5


Это было использование пайпа при помощи bash. Можно проделать все то же самое на С, но для этого нам понадопится еще один системный вызов - `dup`.

```
#include <unistd.h>

/* Возвращает копию нового файлового дескриптора, при этом, по аналогии
   с open, численное значение нового файлового дескриптора - минимальный
   не занятый номер. */
int dup(int old_fd);

/* Создаёт копию нового файлового дескриптора с явно указанным номером new_fd.
   Если ранее файловый дескриптор new_fd был открыт, то закрывает его. */
int dup2(int old_fd, int new_fd);
```


```python
%%file fork_exec_pipe.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
    int fd[2];
    pipe(fd); // fd[0] - in, fd[1] - out (like stdin=0, stdout=1)
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[1]);
    
    if ((pid_2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execlp("tail", "tail", "-n", "5", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]);
    
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

```

    Overwriting fork_exec_pipe.c



```bash
%%bash
gcc fork_exec_pipe.c -o fork_exec_pipe
./fork_exec_pipe
```

    root        9230  0.0  0.0      0     0 ?        I    11:39   0:00 [kworker/3:2]
    xxeniash    9231  0.0  0.0   9940  3600 ?        S    11:39   0:00 bash
    xxeniash    9238  0.0  0.0   2620   956 ?        S    11:39   0:00 ./fork_exec_pipe
    xxeniash    9239  0.0  0.0  12792  3756 ?        R    11:39   0:00 ps aux
    xxeniash    9240  0.0  0.0   8400   980 ?        S    11:39   0:00 tail -n 5


**Важно!** При выполнении системных вызовов `fork`, `dup` или `dup2` создаются копии файловых дескрипторов, связанных с каналом. Если не закрывать все лишние (неиспользуемые) копии файловых дескрипторов, предназначенных для записи, то это приводит к тому, что при очередной попытке чтения из канала, `read` вместо того, чтобы завершить работу, будет находиться в ожидании данных. Т.е. возникнет ситуация deadlock.

## Eventfd

`Eventfd` - механизм для ожидания и отправки сообщений о событиях. Может быть использовано пользовательскими приложениями или ядром для оповещения других пользовательских приложений (но не ядра). Внутри себя содержит счетчик `uint64_t`, поддерживаемый ядром. Максимальное значение, которое может храниться в этом счетчике - это `MAX_INT64 - 1`, т.е. 0xfffffffffffffffe.

Создается с помощью функции:
```
#include <sys/eventfd.h>

int eventfd(unsigned int initval, int flags);
```

Поддерживаются флаги `EFD_CLOEXEC`, `EFD_NONBLOCK` и `EFD_SEMAPHORE`. Они определяют дальнейшее поведение `eventfd`.

`eventfd` по факту является файловым дескриптором, поэтому к нему применимы стандартные системные вызовы `read` и `write`


### Read
Возвращает `uint64_t`.

* Если `EFD_SEMAPHORE` не был выставлен и внутренний счетчик имеет ненулевое значение, то `read` вернет значение этого счетчика, а в сам счетчик запишет 0
* Если `EFD_SEMAPHORE` был выставлен и внутренний счетчик имеет ненулевое значение, то `read` вернет 1 и уменьшит значение счетчика на 1
* Если значение внутреннего счетчика в момент вызова `read` нулевое, то либо `read` будет ждать, пока счетчик станет ненулевым, либо, если `EFD_NONBLOCK` был выставлен, завершится с ошибкой EAGAIN.

### Write
Добавляет 8байтовое число, указанное в буфере, к внутреннему счетчику `eventfd`. Если такое прибавление приведет к переполнению, то `eventfd` либо заблокируется до следующего чтения, либо, если был выставлен флаг `EFD_NONBLOCK` при создании, завершится с ошибкой EAGAIN.

`write` может завершиться с ошибкой, если ему передан буфер размера меньше 8 байт либо если в этом буфере записано 0xffffffffffffffff.

### Пример
В этом примере дочерний процесс пишет в `eventfd`, а родительский оттуда читает.


```python
%%file eventfd.c

#include <err.h>
       #include <inttypes.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <sys/eventfd.h>
       #include <sys/types.h>
       #include <unistd.h>

       int
       main(int argc, char *argv[])
       {
           int       efd;
           uint64_t  u;
           ssize_t   s;

           if (argc < 2) {
               fprintf(stderr, "Usage: %s <num>...\n", argv[0]);
               exit(EXIT_FAILURE);
           }

           efd = eventfd(0, 0);
           if (efd == -1)
               err(EXIT_FAILURE, "eventfd");

           switch (fork()) {
           case 0:
               for (size_t j = 1; j < argc; j++) {
                   printf("Child writing %s to efd\n", argv[j]);
                   u = strtoull(argv[j], NULL, 0);
                           /* strtoull() allows various bases */
                   s = write(efd, &u, sizeof(uint64_t));
                   if (s != sizeof(uint64_t))
                       err(EXIT_FAILURE, "write");
               }
               printf("Child completed write loop\n");

               exit(EXIT_SUCCESS);

           default:
               sleep(2);

               printf("Parent about to read\n");
               s = read(efd, &u, sizeof(uint64_t));
               if (s != sizeof(uint64_t))
                   err(EXIT_FAILURE, "read");
               printf("Parent read %"PRIu64" (%#"PRIx64") from efd\n", u, u);
               exit(EXIT_SUCCESS);

           case -1:
               err(EXIT_FAILURE, "fork");
           }
       }
```

    Overwriting eventfd.c



```bash
%%bash
gcc eventfd.c -o eventfd
./eventfd 1 2 4 7 14
```

    Child writing 1 to efd
    Child writing 2 to efd
    Child writing 4 to efd
    Child writing 7 to efd
    Child writing 14 to efd
    Child completed write loop
    Parent about to read
    Parent read 28 (0x1c) from efd


## Сигналы

Сигнал - механизм передачи коротких сообщений (сообщение состоит из одного чиса - номера сигнала), как правило, прерывающий работу процесса, которому он был отправлен.

Вы, вероятно уже сталкивались со следующими сигналами:
* SIGKILL - посылали его, когда убивали процесс: `kill -9 <pid>`
* SIGINT (~interrupt) - посылается, когда вы нажимаете Ctrl+C для завершения программы
* SIGQIT - посылается, если завершить программу с помощью Ctrl+\. Это завершение программы с дампом памяти. Срабатывает чаще, чем Ctrl+C
* SIGTSTP - посылается, если завершить программу с помощью Ctrl+Z, останавливает процесс

### Обработка сигналов
С каждым процессом связан аттрибут, который не наследуется при `fork`, - это *маска сигналов, ожидающих доставки*. Как правило, она представляется внутри системы в виде целого числа, хотя стандартом внутреннее представление не регламентируется. Отдельные биты в этой маске соответствуют отдельным сигналам, которые были отправлены процессу, но ещё не обработаны.

Поскольку одним битом можно закодировать только бинарное значение, то учитывается только сам факт поступления сигнала, но не их количество. Например, это может быть критичным, если сигналы долго не обрабатываются. Таким образом, использовать механизм стандартных сигналов для синхронизации двух процессов - нельзя.

Тот факт, что сигнал оказался в маске ожидающих доставки, ещё не означает, что он будет немедленно обработан. У процесса (или даже у отдельной нити) может существовать маска *заблокированных* сигналов, которая накладывается на маску ожидающих доставки с помощью поразрядной операции `И-НЕ`.

В отличии от маски ожидающих достаки, маска заблокированных сигналов наследуется при `fork`.

### Множества сигналов

Множества сигналов описываются типом данных `sigset_t`, объявленным в заголовочном файле `<signal.h>`.

Операции над множествами:
 * `sigemptyset(sigset_t *set)` - инициализировать пустое множество;
 * `sigfillset(sigset_t *set)` - инициализировать полное множество;
 * `sigaddset(sigset_t *set, int signum)` - добавить сигнал к множеству;
 * `sigdelset(sigset_t *set, int signum)` - убрать сигнал из множества;
 * `sigismember(sigset_t *set, int signum)` - проверить наличие сигнала в множестве.
 
 Для блокировки или разблокировки отдельных сигналов поддерживается специальная маска. Она устанавливается с помощью системного вызова `sigprocmask`.
 
```
int sigprocmask(int how, sigset_t *set, sigset_t *old_set);
```
где `old_set` - куда записать старую маску (может быть `NULL`, если не интересно), а параметр `how` - это одно из значений:
 * `SIG_SETMASK` - установить множество сигналов в качестве маски блокируемых сигналов;
 * `SIG_BLOCK` - добавить множество к маске блокируемых сигналов;
 * `SIG_UNBLOCK` - убрать множество из маски блокируемых сигналов.




По умолчанию, все сигналы, кроме `SIGCHILD` (информирование о завершении дочернего процесса) и `SIGURG` (информирование о поступлении TCP-сегмента с приоритетными данными), приводят к завершению работы процесса. Но при желании можно написать свой обработчик для тех сигналов, которые можно перехватить, т.е. всех, кроме `SIGSTOP` и `SIGKILL`.

Системный вызов `signal` предназначен для того, чтобы зарегистрировать функцию в качестве обработчика определенного сигнала. Первым аргументом является номер сигнала, вторым - указатель на функцию, которая принимает единственный аргумент - номер пришедшего сигнала (т.е. одну функцию можно использовать сразу для нескольких сигналов), и ничего не возвращает.

```
#include <signal.h>

// Этот тип определен только в Linux!
typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler); // для Linux
void (*signal(int signum, void (*func)(int))) (int); // по стандарту POSIX
```

Частным случаем обработки сигналов является игнорирование сигналов, которое так же можно провести с помощью `signal`.


```python
%%file ignore.c

#include <signal.h>
#include <stdio.h>
 
int main(void)
{
    /* ignoring the signal */
    signal(SIGTERM, SIG_IGN);
    raise(SIGTERM);
    printf("Exit main()\n");
}
```

    Writing ignore.c



```bash
%%bash 
gcc ignore.c -o ignore
./ignore
```

    Exit main()


### Signalfd

`signalfd` создает файловый дескриптор для получения сигналов.

**Как это может быть использовано?** Допустим, мы хотим одновременно дождаться сигнала и какого-то другого события. Ожидать события по нескольким файловым дескрипторам можно при помощи средств мультиплексирования - `select`, `poll`, `epoll`. Значит, если бы у нас получилось свести ожидание сигнала к файловому дескриптору, то его можно было бы обработать одновременно с другими событиями. Здесь и помогает `signalfd`!

```
#include <sys/signalfd.h>

int signalfd(int fd, const sigset_t *mask, int flags);
```

### Пример


```python
%%file signalfd.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signalfd.h>

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGCONT);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    // сводим получение сигналов к файловому дескриптору
    int fd = signalfd(-1, &mask, 0);
    
    struct signalfd_siginfo fdsi;
    while (1) {
        read(fd, &fdsi, sizeof(struct signalfd_siginfo));
        printf("Got signal %d\n", fdsi.ssi_signo);
        if (fdsi.ssi_signo == SIGTERM) {
            printf(" ... and it is SIGTERM\n");
            break;
        }
    }
    close(fd);
    return 0;
}
```

    Writing signalfd.c


Запуск:

```
gcc signalfd.c -o signalfd
timeout -s SIGINT 1 timeout -s SIGTERM 2 timeout -s SIGINT 3  ./signalfd
```

Можно запустить и убедиться в том, что первый SIGINT был проигнорирован, а после получения SIGTERM программа завершилась, поэтому второй SIGINT уже не был доставлен.


```python

```