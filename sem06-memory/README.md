# Работа с памятью


## Краткое повторение лекции

**Основное про память в ОС**:
* Физическая память делится на куски разного размера — сегменты
* За каждым процессом закрепляются несколько сегментов: сегмент с кодом, сегмент с данными, сегмент со стеком итд
* Обращения между сегментами контролируются ОС

**Виртуальная память и страничная адресация*
* Вся физическая память делится на *фреймы* - куски равного размера (4096 байт на x86)
* Каждому процессу выделяется свое *изолированное 64-битное адресное пространство*
* Эта *виртуальная память* делится на **страницы** аналогично фреймам
* Каждой странице а адресном пространстве может соответствовать какой-то фрейм

Подробнее про виртуальную память, страничную адресацию (в частности, hierarchical page tables) - на лекции.

## Page fault
Как правило, память выделяется с помощью `mmap()` или `malloc()`. По умолчанию, ядро не сразу выделяет физические страницы для каждого запроса на выделение памяти. Иногда это приводит к *page fault* -  ситуации, когда мы обращаемся к странице памяти без должной подготовки.

**Minor page fault** - обращение к странице, которая уже присутствует в RAM, но у процессора еще нет правильного отображения (mapping) на эту страницу. Если у процессора достаточно свободной памяти, то обращение к буферам выделенным с помощью `malloc` может быть обработано с `minor page fault`.

**Major page fault** - обращение к странице, которая еще не выгружена в RAM. Например, такое может быть, если обратиться к *swapped-out* страницам, или при чтении из файла, который только что был отображен на память с помощью `mmap`.


```python
%%file read_to_buff.cpp

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
    assert(argc == 3);
    int buff_size = 1;
    
    int ret = sscanf(argv[1], "%d", &buff_size);
    assert(ret == 1);
    
    int fd = open(argv[2], O_RDONLY);
    assert(fd >= 0);
    
    char buff[buff_size];
    
    int result = 0;
    int cnt = 0;
    
    while ((cnt = read(fd, buff, buff_size)) > 0) {
        for (int i = 0; i < cnt; ++i) {
            result += buff[i];
        }
    }
    printf("CNT: %d\n", cnt);
    return 0;
}
```

    Writing read_to_buff.cpp



```bash
%%bash
for i in {0..1000000} ; do echo -n '1' ; done > input.txt
```


```bash
%%bash
gcc read_to_buff.cpp -o read_to_buff
```


```bash
%%bash
wc -c input.txt
```

    1000001 input.txt



```bash
%%bash
\time ./read_to_buff 1 input.txt
```

    CNT: 0


    0.04user 0.15system 0:00.19elapsed 100%CPU (0avgtext+0avgdata 1564maxresident)k
    0inputs+0outputs (0major+74minor)pagefaults 0swaps



```bash
%%bash
\time ./read_to_buff 100 input.txt
```

    CNT: 0


    0.00user 0.00system 0:00.00elapsed 100%CPU (0avgtext+0avgdata 1684maxresident)k
    0inputs+0outputs (0major+73minor)pagefaults 0swaps


## Мониторинг с помощью perf
Установка:
```
sudo apt-get install linux-tools-common linux-tools-generic linux-tools-`uname -r`
```
Использование:
```
sudo -S perf stat ./read_to_buff 1000 input.txt 2>&1 
```

После запуска последней команды получаем такой результат:
```
CNT: 0

 Performance counter stats for './read_to_buff 1000 input.txt':

              2,75 msec task-clock                #    0,888 CPUs utilized          
                 0      context-switches          #    0,000 /sec                   
                 0      cpu-migrations            #    0,000 /sec                   
                62      page-faults               #   22,577 K/sec                  
         9 305 034      cycles                    #    3,388 GHz                    
        12 609 029      instructions              #    1,36  insn per cycle         
         1 510 267      branches                  #  549,962 M/sec                  
             7 977      branch-misses             #    0,53% of all branches        

       0,003091555 seconds time elapsed

       0,003152000 seconds user
       0,000000000 seconds sys
```

## Кэши процессора
**Два принципа локальности кэшей:**
* *Temporal locality*: если процесс прочитал какую-то память, то скорее всего, скоро он прочитает её ещё раз
* *Spatial locality*: если процесс прочитал какую-то память, то, скорее всего, скоро он прочитает память следующую за ней

**Основное о кэшах процессора:**
* Мало данных, очень быстрый доступ
* Кэши иерархичны: L1, L2, L3
* Обычно из памяти зачитывается сразу кэш-линия (64 байта)
* LRU (least recently used) для вытеснения данных

**Об уровнях кэшей:**
* L1 кэш: per-core кэш, обычно разделён на кэш инструкций (L1i) и кэш данных (L2d), доступ: low-priority цикла, ~0.5 ns
* L2 кэш: больше по размеру, может разделяться на несколько ядер, доступ: ~12 циклов, ~4-7 ns
* L3 кэш: ещё больше по размеру (1-8 Mb), обычно один на процессор, доступ: ~40-80 циклов, ~12-20 ns
* Доступ к DRAM (если известен физический адрес): 50-100 циклов, 16+ ns

В этом [обсуждении](https://stackoverflow.com/questions/21369381/measuring-cache-latencies) можно почитать про то, как побенчмаркать кэш самостоятельно.

Средние значения:
```
L1 cache reference                           0.5 ns
Branch mispredict                            5   ns
L2 cache reference                           7   ns                      14x L1 cache
Mutex lock/unlock                           25   ns
Main memory reference                      100   ns                      20x L2 cache, 200x L1 cache
Compress 1K bytes with Snappy            3,000   ns        3 µs
Read 1 MB sequentially from memory      20,000   ns       20 µs          ~50GB/sec DDR5
Read 1 MB sequentially from NVMe       100,000   ns      100 µs          ~10GB/sec NVMe, 5x memory
Round trip within same datacenter      500,000   ns      500 µs
Read 1 MB sequentially from SSD      2,000,000   ns    2,000 µs    2 ms  ~0.5GB/sec SSD, 100x memory, 20x NVMe
Read 1 MB sequentially from HDD      6,000,000   ns    6,000 µs    6 ms  ~150MB/sec 300x memory, 60x NVMe, 3x SSD
Send 1 MB over 1 Gbps network       10,000,000   ns   10,000 µs   10 ms
Disk seek                           10,000,000   ns   10,000 µs   10 ms  20x datacenter roundtrip
Send packet CA->Netherlands->CA    150,000,000   ns  150,000 µs  150 ms
```

## Системный вызов mmap
(взято из [ридинга Яковлева](https://github.com/victor-yacovlev/fpmi-caos/tree/master/practice/mmap) )


```c
#include <sys/mman.h>

void *mmap(
    void *addr,    /* рекомендуемый адрес отображения. Должен быть выровнен! */
    size_t length, /* размер отображения */
    int prot,      /* аттрибуты доступа */
    int flags,     /* флаги совместного отображения */
    int fd,        /* файловый декскриптор файла */
    off_t offset   /* смещение относительно начала файла. Должен быть выровнен! */
  );

int munmap(void *addr, size_t length) /* освободить отображение */
```

Системный вызов `mmap` предназначен для создания в виртуальном адресном пространстве процесса доступной области по определенному адресу. Эта область может быть как связана с определенным файлом (ранее открытым), так и располагаться в оперативной памяти. Второй способ использования обычно реализуется в функциях `malloc`/`calloc`.

Память можно выделять только постранично. Для большинства архитектур размер одной страницы равен 4Кб, хотя процессоры архитектуры x86_64 поддерживают страницы большего размера: 2Мб и 1Гб.

В общем случае, никогда нельзя полагаться на то, что размер страницы равен 4096 байт. Его можно узнать с помощью команды `getconf` или функции `sysconf`:

```bash
# Bash
> getconf PAGE_SIZE
4096
```
```c
/* Си */
#include <unistd.h>
long page_size = sysconf(_SC_PAGE_SIZE);
```

Параметр `offset` (если используется файл) обязан быть кратным размеру страницы; параметр `length` - нет, но ядро системы округляет это значение до размера страницы в большую сторону. Параметр `addr` (рекомендуемый адрес) может быть равным `NULL`, - в этом случае ядро само назначает адрес в виртуальном адресном пространстве.

При использовании отображения на файл, параметр `length` имеет значение длины отображаемых данных; в случае, если размер файла меньше размера страницы, или отображается его последний небольшой фрагмент, то оставшаяся часть страницы заполняется нулями.

Страницы памяти могут флаги аттрибутов доступа:
 * чтение `PROT_READ`;
 * запись `PROT_WRITE`;
 * выполнение `PROT_EXE`;
 * ничего `PROT_NONE`.

В случае использования отображения на файл, он должен быть открыт на чтение или запись в соответствии с требуемыми аттрибутами доступа.

Флаги `mmap`:
 * `MAP_FIXED` - требует, чтобы память была выделена по указаному в первом аргументе адресу; без этого флага ядро может выбрать адрес, наиболее близкий к указанному.
 * `MAP_ANONYMOUS` - выделить страницы в оперативной памяти, а не связать с файлом.
 * `MAP_SHARED` - выделить страницы, разделяемые с другими процессами; в случае с отображением на файл - синхронизировать изменения так, чтобы они были доступны другим процессам.
 * `MAP_PRIVATE` - в противоположность `MAP_SHARED`, не делать отображение доступным другим процессам. В случае отображения на файл, он доступен для чтения, а созданные процессом изменения, в файл не сохраняются.





```python
%%file mmap_example.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

int get_page_size() {
    static int page_size = 0;
    return page_size = page_size ?: sysconf(_SC_PAGE_SIZE);
}

int upper_round_to_page_size(int sz) {
    return (sz + get_page_size() - 1) / get_page_size() * get_page_size();
}

int main() {
    printf("page size = %d\n", get_page_size());
    int fd = open("buf.txt", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    
    printf("file size = %d\n", (int)s.st_size);
    int old_st_size = s.st_size;
    if (s.st_size < 2) {
        const int new_size = 10;
        assert(ftruncate(fd, new_size) == 0); // изменяем размер файла
        assert(fstat(fd, &s) == 0);
        printf("new file size = %d\n", (int)s.st_size);
    }
    
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ s.st_size, 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE,
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    assert(mapped != MAP_FAILED);
    assert(close(fd) == 0); // Не забываем закрывать файл
    
    char* buf = mapped;
    
    if (old_st_size != s.st_size) {
        for (int j = old_st_size; j < s.st_size; ++j) {
            buf[j] = '_';
        }
    }
    
    buf[1] = ('0' <= buf[1] && buf[1] <= '9') ? ((buf[1] - '0' + 1) % 10 + '0') : '0';
    
    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ s.st_size
    ) == 0);
    
    return 0;
}
```

    Writing mmap_example.c



```bash
%%bash
gcc mmap_example.c -o mmap_example
echo "0000" > buf.txt && ./mmap_example && cat buf.txt
```

    page size = 4096
    file size = 5
    0100


## Procfs
`Procfs` (Process Filesystem) - виртуальная файловая система, которая используется процессами для того, чтобы хранить информацию о себе.

Точка монтирования этой файловой системы - `/proc`.

Посмотрим пару примеров ее использования.

* `/proc/meminfo` - информация об используемой памяти
* `/proc/modules` - информация о текущих модулях ядра
* `/proc/devices` - зарегистрированные символьный и блочные устройства
* `/proc/iomem` - физические адреса RAM и шин памяти
* `/proc/swaps` - активные на текущий момент свопы
* `/proc/filesystems` - активные драйвера файловых систем
* `/proc/cpuinfo` - информация про CPU

Узнать, что еще там лежит, можно с помощью `ls /proc`.

## Инструменты для отладки при работе с памятью
При работе с памятью возникают разные проблемы: проезд по памяти, утечки памяти, UP, page fault (проезд по памяти, который приводит к тому, что мы обращаемся к странице, на которую не имеем прав).

Для отладки программы в общем случае используем `gdb`.

**Полезные санитайзеры**:
* AddressSanitizer (`fsanitize=address`) - диагностирует ситуации утечек памяти, двойного освобождения памяти, выхода за границу стека или кучи, и использования указателей на стек после завершения работы функции.
* MemorySanitizer (`-fsanitize=memory`) - диагностика ситуаций чтения неинициализированных данных. Требует, чтобы программа, и как и все используемые ею библиотеки, были скомпилированы в позиционно-независимый код.
* UndefinedBehaviourSanitizer (`-fsanitize=undefined`) - диагностика неопределенного поведения в целочисленной арифметике: битовые сдвиги, знаковое переполнение, и т.д.

Для работы с утечками памяти используем `valgrind`.

Для отладки системных вызовов используется `strace`.

Сегодня так же познакомились с профилировщиком `perf`.


