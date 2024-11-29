# Библиотеки функций и их загрузка

## Функции и указатели на них
Код программ в системах с Фон-Неймановской архитектурой размещается в памяти точно так же, как и обычные данные.

Таким образом, он может быть загружен или сгенерирован во время работы программы. Некоторые процессоры позволяют контролировать, какие участки памяти могут быть выполняемые, а какие - нет, и кроме того, это контролируется ядром. Таким образом, выполнить код можно только при условии, что он находится в страницах памяти, помеченных как выполняемые.

Поэтому существует такое понятие, как указатель на функцию.

Общий вид указателя на функцию:
```
typedef ResType (*TypeName)(FuncParameters...);
```

Конкретный пример:
```
int (*p_function)(int a, int b);
```

## Библиотеки
 * содержит таблицу доступных *символов* - функций и глобальных переменных (можно явно указать её создание опцией `-E`);
 * может быть размещена произвольным образом, поэтому программа обязана быть скомпилирована в позиционно-независимый код с опцией `-fPIC` или `-fPIE`;
 * не обязана иметь точку входа в программу - функции `_start` и `main`.

Компиляция библиотеки производится с помощью опции `-shared`:
```
 > gcc -fPIC -shared -o libmy_great_library.so lib.c
```

В Linux и xBSD для именования библиотек используется соглашение `libИМЯ.so`, для Mac - `libИМЯ.dynlib`, для Windows - `ИМЯ.dll`.

Связывание программы с библиотекой подразумевает опции:
 * `-lИМЯ` - указыватся имя библиотеки без префикса `lib` и суффикса `.so`;
 * `-LПУТЬ` - указывается имя каталога для поиска используемых библиотек.

## Позиционно-независимый исполняемый файл

Опция `-fPIE` компилятора указывает на то, что нужно сгенерировать позиционно-независимый код для `main` и `_start`, а опция `-pie` - о том, что нужно при линковке указать в ELF-файле, что он позиционно-независимый.

Позиционно-независимый выполняемый файл в современных системах размещается по случайному адресу.

Если позиционно-независимый исполняемый файл ещё и содержит таблицу экспортируемых символов, то он одновременно является и библиотекой. Если отсутствует опция `-shared`, то компилятор собирает программу, удаляя из неё таблицу символов. Явным образом сохранение таблицы символов задается опцией `-Wl,-E`.

Пример:
```
  # файл abc.c содержит int main() { puts("abc"); }
  > gcc -o program -fPIE -pie -Wl,-E abc.c

  # программа может выполняться как обычная программа
  > ./program
  abc

  # и может быть использована как библиотека
  > python3
  >>> from ctypes import cdll, c_int
  >>> lib = cdll.LoadLibrary("./program")
  >>> main = lib["main"]
  >>> main.restype = c_int
  >>> ret = main()
  abc  

 ```
 
 ## Примеры
 Рассмотрим базовый случай того, что такое библиотека. 


```python
%%file foo.h

int foo(int a);
```

    Overwriting foo.h



```python
%%file foo.c

int foo(int a) {
    return a + 42;
}
```

    Writing foo.c



```python
%%file main.c

#include <stdio.h>
#include "foo.h"

int main() {
    int a = -41;
    printf("%d\n", foo(a));
}
```

    Overwriting main.c


Как собрать это вместе?

### Способ 1. Просто вместе скомпилировать


```bash
%%bash
gcc main.c foo.c -o simple_var
./simple_var
```

    1


Но что, если мы хотим один раз скомпилировать реализацию функции `foo` (представим, что она долго компилируется) и потом просто переиспользовать готовый код? Для этого существуют статические и динамические библиотеки.

### Способ 2. Статические библиотеки


```bash
%%bash
gcc -c foo.c -o foo.o # создаем объектный файл
ar rcs libfoo.a foo.o # создаем статическую библиотеку libfoo
gcc main.c -L. libfoo.a -o with_static # компилируем с main.c
./with_static
```

    1


`-L` указывает путь до директории, в которой нужно искать код подгружаемой библиотеки.

Код статической библиотеки при линковке будет полностью скопирован в код итоговой программы. Если бы все библиотеки подгружались статически, то было бы много дубликатов одного и того же кода, т.е. память бы использовалась совершенно неэффективно. Решение этой проблемы - *динамические (разделяемые)* библиотеки.

### Способ 3. Динамические библиотеки


```bash
%%bash
gcc -Wall -shared -fPIC foo.c -o libfoo.so
```

Для того, чтобы линкер нашел нашу библиотеку `lobfoo.so`, нужно указать путь до директории, содержащей его, в переменной `LD_LIBRARY_PATH`.


```bash
%%bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH://home/xxeniash/os-seminars/sem12-13-libs-isolation/
echo $LD_LIBRARY_PATH
gcc main.c -L. -lfoo -o with_shared
./with_shared
```

    ://home/xxeniash/os-seminars/sem12-13-libs-isolation/
    1


Мы разобрались с двумя видами библиотек. Теперь посмотрим еще на другие интересные способы их использования

### Подгрузка динамической библиотеки из python


```python
%%file sum.c

int sum_int(int a, int b) {
    return a + b;
}

float sum_float(float a, float b) {
    return a + b;
}
```

    Overwriting sum.c


Соберем это в динамическую библиотеку.


```bash
%%bash
gcc -Wall -shared -fPIC sum.c -o libsum.so
```

Теперь воспользуемся этой библиотекой для вычислений из кода на python.


```python
import ctypes

lib = ctypes.CDLL("./libsum.so")
print(lib.sum_int(3, 4)) # По умолчанию считает типы int'ами, поэтому в этом случае все хорошо
print(lib.sum_float(3, 4)) # А здесь python передает в функцию инты, а она принимает float'ы. Тут может нарушаться соглашение о вызовах и происходить что угодно

# Скажем, какие на самом деле типы в сигнатуре функции
lib.sum_float.argtypes = [ctypes.c_float, ctypes.c_float]
lib.sum_float.restype = ctypes.c_float
print(lib.sum_float(3, 4)) # Теперь все работает хорошо
```

    7
    0
    7.0


## Таблицы символов

Посмотрим на таблицы символов для двух только что скомпилированных библиотек.


```bash
%%bash
objdump -t libfoo.so 
```

    
    libfoo.so:     file format elf64-x86-64
    
    SYMBOL TABLE:
    0000000000000000 l    df *ABS*	0000000000000000 crtstuff.c
    0000000000001040 l     F .text	0000000000000000 deregister_tm_clones
    0000000000001070 l     F .text	0000000000000000 register_tm_clones
    00000000000010b0 l     F .text	0000000000000000 __do_global_dtors_aux
    0000000000004020 l     O .bss	0000000000000001 completed.0
    0000000000003e88 l     O .fini_array	0000000000000000 __do_global_dtors_aux_fini_array_entry
    00000000000010f0 l     F .text	0000000000000000 frame_dummy
    0000000000003e80 l     O .init_array	0000000000000000 __frame_dummy_init_array_entry
    0000000000000000 l    df *ABS*	0000000000000000 foo.c
    0000000000000000 l    df *ABS*	0000000000000000 crtstuff.c
    00000000000020a0 l     O .eh_frame	0000000000000000 __FRAME_END__
    0000000000000000 l    df *ABS*	0000000000000000 
    0000000000003e90 l     O .dynamic	0000000000000000 _DYNAMIC
    0000000000004020 l     O .data	0000000000000000 __TMC_END__
    0000000000004018 l     O .data	0000000000000000 __dso_handle
    0000000000001000 l     F .init	0000000000000000 _init
    0000000000002000 l       .eh_frame_hdr	0000000000000000 __GNU_EH_FRAME_HDR
    000000000000110c l     F .fini	0000000000000000 _fini
    0000000000004000 l     O .got.plt	0000000000000000 _GLOBAL_OFFSET_TABLE_
    0000000000000000  w      *UND*	0000000000000000 __cxa_finalize
    0000000000000000  w      *UND*	0000000000000000 _ITM_registerTMCloneTable
    0000000000000000  w      *UND*	0000000000000000 _ITM_deregisterTMCloneTable
    00000000000010f9 g     F .text	0000000000000013 foo
    0000000000000000  w      *UND*	0000000000000000 __gmon_start__
    
    



```bash
%%bash
objdump -t libfoo.so | grep foo
objdump -t libsum.so | grep sum
```

    libfoo.so:     file format elf64-x86-64
    0000000000000000 l    df *ABS*	0000000000000000 foo.c
    00000000000010f9 g     F .text	0000000000000013 foo
    libsum.so:     file format elf64-x86-64
    0000000000000000 l    df *ABS*	0000000000000000 sum.c
    0000000000001111 g     F .text	000000000000001e sum_float
    00000000000010f9 g     F .text	0000000000000018 sum_int


## Загрузка динамической библиотеки из программы на С, используя стандартные функции
Библиотеки можно не привязывать намертво к программе, а загружать по мере необходимости. Для этого используется набор функций `dl`, которые вошли в стандарт POSIX 2001 года.

 * `void *dlopen(const char *filename, int flags)` - загружает файл с библиотекой;
 * `void *dlsym(void *handle, const char *symbol)` - ищет в библиотеке необходимый символ, и возвращает его адрес;
 * `int dlclose(void *handle)` - закрывает библиотеку, и выгружает её из памяти, если она больше в программе не используется;
 * `char *dlerror()` - возвращает текст ошибки, связянной с `dl`.



```python
%%file stdload_exec_dynlib.c

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
```

    Overwriting stdload_exec_dynlib.c



```bash
%%bash
gcc -Wall -g stdload_exec_dynlib.c -ldl -o stdload_exec_dynlib
./stdload_exec_dynlib
```

    sum(1, 1) = 2
    sum(40, 5000) = 5040
    sum_f(1, 1) = 2.00
    sum_f(4.0, 500.1) = 504.10


# BPF
[Ридинг Яковлева](https://github.com/victor-yacovlev/fpmi-caos/tree/master/practice/bpf)

**BPF (Berkeley Packet Filter)** -  механизм для захвата и фильтрации пакетов на уровне ядра. Оперирует на уровне Data Link Layer. Мотивация к его созданию была такой - хочется отфильтровывать лишние пакеты и избавиться от них как можно раньше.

Первая версия BPF сейчас называется classic BPF (`cBPF`) и включает в себя, кроме прочего, следующие утилиты:
* `tcpdump` (ее мы использовали для захвата и логирования трафика на прошлых семинарах)
* `seccomp` - механизм для фдра Linux, позволяющий процессам определять системные вызовы, которые они будут использовать (используется для исполнения "серых" программ)

Extended BPF или eBPF - усовершенствованная версия cBPF, оптимизированная под 64битные машины. В своей основе eBPF - виртуальная машина-песочница, позволяющая запускать "произвольный" код без ущерба для безопасности.

Программы BPF создаются в пространстве пользователя, загружаются в ядро и подсоединяются к какому-нибудь источнику событий. Событием может быть, например, доставка пакета на сетевой интерфейс, запуск какой-нибудь функции ядра, и т.п. В случае пакета программе BPF будут доступны данные и метаданные пакета (на чтение и, может, на запись, в зависимости от типа программы), в случае запуска функции ядра — аргументы функции, включая указатели на память ядра, и т.п.

Если программы для cBPF нужно было обязательно писать на ассемблере, то eBPF принимает программы на С, и делается это за счет использования llvm. 


```python

```