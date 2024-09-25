# Файловые системы
Сегодня рассмотрим следующие системные вызовы:

## О связи процессов в системе и файлов
Каждому открытому в процессе файлу соответствует число (int) - *файловый дескриптор*.

Фиксированные номера для файловых дексрипторов:
* 0 - stdin - стандартный поток ввода (STDIN_FILENO - стандартный макрос в C)
* 1 - stdout - стандартный поток вывода (STDOUT_FILENO)
* 2 - stderr - стандартный поток ошибок (STDERR_FILENO)

Остальным открытым файлам соответствуют числа 3, 4 и т.д.

Стандартные потоки ввода, вывода и ошибок можно перенаправлять:

* `grep String < file.txt` <-> `grep String 0< file.txt`
* `mkdir a_dir 2> /dev/null`
* `./some_program < in.txt 1> out.txt` <-> `./some_program < in.txt > out.txt` 

## Open and Read
Открытие файла:
```
#include <fcntl.h>
int open(const char *pathname, int flags, ... /* mode_t mode */ );
```
Чтение файла:
```
#include <unistd.h>

ssize_t read(int fd, void buf[.count], size_t count);
ssize_t pread(int fd, void buf[.count], size_t count, off_t offset); 
ssize_t readv(int fd, const struct iovec *iov, int iovcnt); // 

```
`pread()` - чтение начиная с позиции `offset` \
`readv()` - чтение в несколько буфферов, заданных `iov`.




```bash
%%bash
cat read.c
```

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <assert.h>
    
    int main(int argc, char* argv[]) {
        assert(argc >= 2);
        int fd = open(argv[1], O_RDONLY);
    
        if (fd < 0) {
            fprintf(stderr, "%s", "Error opening file!\n");
        }
        
        char buffer[4096];
        int bytes_read = read(fd, buffer, sizeof(buffer));
        
        if (bytes_read < 0) {
            fprintf(stderr, "%s", "Error reading file!\n");
        }
        
        if (bytes_read < sizeof(buffer)) {
            buffer[bytes_read] = 0;
        }
    
        printf("Read %d bytes: %s\n", bytes_read, buffer);
    
        return 0;
    }



```bash
%%bash
gcc read.c -o read
echo 'Reading read.txt:'
./read read.txt
echo 'Reading file that does not exist:'
./read no_file.txt
```

    Reading read.txt:
    Read 13 bytes: hello world!
    
    Reading file that does not exist:


    Error opening file!
    Error reading file!


    Read -1 bytes: 0


Read может считать меньше байт, чем мы у него запрашиваем, если они на текущий момент не доступны. Для того, чтобы считать именно это количество байт, нужно повторить запрос.

## Write
Запись в файл:
```
#include <unistd.h>

ssize_t write(int fd, const void buf[.count], size_t count);
ssize_t pwrite(int fd, const void buf[.count], size_t count, off_t offset);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```
`pwrite()` - запись с позиции `offset` \
`writec()` - собирает данные для вывода из нескольких буферов


```bash
%%bash
cat write.c
```

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <assert.h>
    #include <string.h>
    
    int main(int argc, char* argv[]) {
        assert(argc >= 2);
        int fd = open(argv[1], O_WRONLY);
    
        if (fd < 0) {
            fprintf(stderr, "%s", "Error opening file!\n");
        }
        
        const char buffer[] = "Hello world!\n";
        int bytes_written = write(fd, buffer, strlen(buffer));
        
        if (bytes_written < 0) {
            fprintf(stderr, "%s", "Error writing to file!\n");
        }
        
        printf("Wrote %d bytes\n", bytes_written);
    
        return 0;
    }



```bash
%%bash
gcc write.c -o write
echo 'Writing to write.txt: '
./write write.txt
cat write.txt
echo ''
echo 'Writing to no_file.txt: '
./write no_file.txt
```

    Writing to write.txt: 
    Wrote 13 bytes
    Hello world!
    
    Writing to no_file.txt: 


    Error opening file!
    Error writing to file!


    Wrote -1 bytes


С `write` возникает та же проблема, что и с `read`.

## Общий пример


```bash
%%bash
cat read_and_write.c
```

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    
    int main(int argc, char *argv[])
    {
        printf("Linux by printf");
        
        char linux_str[] = "Linux by write\n";
        
        // write
        // 1 = STDOUT_FILENO - изначально открытый файловый дескриптор соответствующий stdout
        // linux_str - указатель на начало данных, 
        // sizeof(linux_str) - размер данных, которые хотим записать
        // ВАЖНО, что write может записать не все данные 
        //        и тогда его надо перезапустить
        //        но в данном примере этого нет
        // Подробнее в `man 2 write`
        write(1, linux_str, sizeof(linux_str) - 1); 
            
        if (argc < 2) {
            fprintf(stderr, "Need at least 2 arguments\n");
            return 1;
        }
        int fd = open(argv[1], O_RDONLY); // открываем файл и получаем связанный файловый дескриптор
                                          // O_RDONLY - флаг о том, что открываем в read-only режиме
                                          // подробнее в `man 2 open`
        if (fd < 0) {
            perror("Can't open file"); // Выводит указанную строку в stderr 
                                       // + добавляет сообщение и последней произошедшей ошибке 
                                       // ошибка хранится в errno
            return -1;
        }
        
        char buffer[4096];
        int bytes_read = read(fd, buffer, sizeof(buffer)); // fd - файловый дескриптор выше открытого файла
                                                           // 2 и 3 аргументы как во write
                                                           // Так же как и write может прочитать МЕНЬШЕ
                                                           //   чем запрошено в 3м аргументе
                                                           //   это может быть связано как с концом файла
                                                           //   так и с каким-то более приоритетным событием
        if (bytes_read < 0) {
            perror("Error reading file");
            close(fd); // закрываем файл связанный с файловым дескриптором. Ну или не файл. 
                       // Стандартные дескрипторы 0, 1, 2 тоже можно так закрывать
            return -1;
        }
        char buffer2[4096];
        // формирование строки с текстом
        int written_bytes = snprintf(buffer2, sizeof(buffer2), 
            "%d bytes read: '''%.*s'''\n", bytes_read, bytes_read, buffer);
        write(1, buffer2, written_bytes);
        close(fd);
        return 0;
    }



```bash
%%bash
gcc read_and_write.c -o read_and_write
./read_and_write read.txt
```

    Linux by write
    11 bytes read: '''read input
    '''
    Linux by printf

## Чтение и запись с произвольной позиции в файле с lseek
```
off_t lseek(int fd, off_t offset, int whence);
```
Варианты для `whence`:
* `SEEK_SET` - от начала файла
* `SEEK_END` - от конца
* `SEEK_CUR` - от текущей позиции



```bash
%%bash
cat lseek.c
```

    #include <assert.h>
    #include <fcntl.h>
    #include <inttypes.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    
    int main(int argc, char* argv[]) {
        assert(argc >= 2);
    
        int fd = open(argv[1], O_RDWR | O_CREAT);
    
        // получим размер файла, переместившись в его конец
        int size = lseek(fd, 0, SEEK_END);
        printf("File size: %d\n", size);
    
        // если размер меньше 2, то допишем строку
        if (size < 2) {
            const char s[] = "ab";
            lseek(fd, 0, SEEK_SET); // перемещаемся в начало
            write(fd, s, sizeof(s) - 1);
    
            size = lseek(fd, 0, SEEK_END);
            printf("New file size: %d\n", size);
        }
    
        // теперь прочитаем второй символ
        lseek(fd, 1, SEEK_SET);
        char c;
        read(fd, &c, 1);
    
        printf("Second char: %c\n", c);
    
        return 0;
    
    }



```bash
%%bash
gcc lseek.c -o lseek
./lseek new.txt
```

    File size: 2
    Second char: b


## Чтение и запись с произвольной позиции в файле с pwread  и pwrite


```bash
%%bash
cat pread_pwrite.c
```

    #include <assert.h>
    #include <fcntl.h>
    #include <inttypes.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    
    int main(int argc, char* argv[]) {
        assert(argc >= 2);
    
        int fd = open(argv[1], O_RDWR | O_CREAT);
    
        // получим размер файла, переместившись в его конец
        int size = lseek(fd, 0, SEEK_END);
        printf("File size: %d\n", size);
    
        // если размер меньше 2, то допишем строку
        if (size < 2) {
            const char s[] = "ab";
            pwrite(fd, s, sizeof(s) - 1, /*offset =  */0);
    
            size = lseek(fd, 0, SEEK_END);
            printf("New file size: %d\n", size);
        }
    
        // теперь прочитаем второй символ
        char c;
        pread(fd, &c, 1, /*offset = */1);
    
        printf("Second char: %c\n", c);
    
        return 0;
    
    }



```bash
%%bash
gcc pread_pwrite.c -o pread_pwrite
./pread_pwrite new.txt
```

    File size: 2
    Second char: b


## Файловая система в Linux
Файловая система Linux - иерархическая, т.е. есть корневая директория (`/`), ее поддиректории, их поддиректории и т.д.

Парадигма Linux: **everything is a file**.

Типы файлов в Linux:
* регулярные
* директории
* символические ссылки
* сокеты и именованные пайпы
* символьные и блочные устройства

*Жесткая ссылка* - дополнительное имя существующего файла. Создание новой жесткой ссылки:
```
ln {source} {link}
```
*Символическая (мягкая) ссылка* - файл, содержащий путь к другому файлу. Создание новой символической ссылки:
```
ln -s {source} {link}
```

## Атрибуты файлов и разные способы их получить
Атрибуты файла хранятся в структуре `stat`.
Описание структуры `stat` из `man 2 stat`:

```c
struct stat {
   dev_t     st_dev;         /* ID of device containing file */
   ino_t     st_ino;         /* inode number */
   mode_t    st_mode;        /* protection */
   nlink_t   st_nlink;       /* number of hard links */
   uid_t     st_uid;         /* user ID of owner */
   gid_t     st_gid;         /* group ID of owner */
   dev_t     st_rdev;        /* device ID (if special file) */
   off_t     st_size;        /* total size, in bytes */
   blksize_t st_blksize;     /* blocksize for filesystem I/O */
   blkcnt_t  st_blocks;      /* number of 512B blocks allocated */

   /* Since Linux 2.6, the kernel supports nanosecond
      precision for the following timestamp fields.
      For the details before Linux 2.6, see NOTES. */

   struct timespec st_atim;  /* time of last access */
   struct timespec st_mtim;  /* time of last modification */
   struct timespec st_ctim;  /* time of last status change */

#define st_atime st_atim.tv_sec      /* Backward compatibility */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};
```

Разные способы получить атрибуты файла:
```c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *pathname, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *pathname, struct stat *buf);

#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>

int fstatat(int dirfd, const char *pathname, struct stat *buf,
		   int flags);
```

Разница между `stat` и `lstat`: если путь `pathname` является путем к символической ссылке, то `stat` вернет атрибуты файла, к которому она ведет, а `lstat` атрибуты самой ссылки.

Особый интерес будет представлять поле `.st_mode`

Биты соответствующие маскам:
* `0170000` - тип файла.

  Эти биты стоит рассматривать как одно число, по значению которого можно определить тип файла. Сравнивая это число с:  
    * `S_IFSOCK   0140000   socket`
    * `S_IFLNK    0120000   symbolic link`
    * `S_IFREG    0100000   regular file`
    * `S_IFBLK    0060000   block device`
    * `S_IFDIR    0040000   directory`
    * `S_IFCHR    0020000   character device`
    * `S_IFIFO    0010000   FIFO`
* `0777` - права на файл.

  Эти биты можно рассматривать как независимые биты, каджый из которых отвечает за право (пользователя, его группы, всех остальных) (читать/писать/выполнять) файл.
  
**Замечание:** имя файла не является его атрибутом.



```bash
%%bash
cat stat.c
```

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdio.h>
    
    void describe_stat_st_mode(const struct stat* s) {   
        printf("is regular: %s    ", ((s->st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
        printf("is directory: %s    ", S_ISDIR(s->st_mode) ? "yes" : "no "); // or predefined macro
        printf("is symbolic link: %s\n", S_ISLNK(s->st_mode) ? "yes" : "no "); 
    };
    
    
    int main(int argc, char *argv[])
    {
        int fd = open(argv[1], O_RDONLY);
        struct stat s;
        fstat(fd, &s); // get stat for stdin
        describe_stat_st_mode(&s); 
        return 0;
    }



```bash
%%bash
gcc stat.c -o stat
./stat new.txt
```

    is regular: yes    is directory: no     is symbolic link: no 


## Работа с директориями
Создание директории:
```c
#include <sys/stat.h>

int mkdir(const char *pathname, mode_t mode);
```
В случае успеха вернет 0, иначе 1.

Работа с директориями:
```c
#include <sys/types.h>
#include <dirent.h>

DIR *opendir(const char *name);
DIR *fdopendir(int fd);
```

Посмотрим, как вывести список файлов в директории.


```bash
%%bash
gcc dir.c -o dir
./dir ..
```

    .git
    ..
    sem01-intro
    sem03-files
    sem02-int-float-string
    .


## Работа со временем
### Текущее время


```
#include <time.h>

time_t time(time_t *_Nullable tloc);
```
`time` возвращает количество секунд, прошедших с начала эпохи (1 января 1970 года).

**Вопрос:** в чем проблема с функцией `time`?

В случае, когда требуется более высокая точность, чем 1 секунда, можно использовать системный вызов `gettimeofday`, который позволяет получить текущее время в виде структуры:
```
struct timeval {
  time_t      tv_sec;  // секунды
  suseconds_t tv_usec; // микросекунды
};
```

В этом случае, несмотря на то, что в структуре определено поле для микросекунд, реальная точность будет составлять порядка 10-20 миллисекунд для Linux.

Более высокую точность можно получить с помощью системного вызова `clock_gettime`.


### Разложение времени на составляющие

Человеко-представимое время состоит из даты (год, месяц, день) и времени суток (часы, минуты, секунды).

Это описывается структурой:
```
struct tm { /* время, разбитое на составляющие */
  int tm_sec; /* секунды от начала минуты: [0 -60] */
  int tm_min; /* минуты от начала часа: [0 - 59] */
  int tm_hour; /* часы от полуночи: [0 - 23] */
  int tm_mday; /* дни от начала месяца: [1 - 31] */
  int tm_mon; /* месяцы с января: [0 - 11] */
  int tm_year; /* годы с 1900 года */
  int tm_wday; /* дни с воскресенья: [0 - 6] */
  int tm_yday; /* дни от начала года (1 января): [0 - 365] */
  int tm_isdst; /* флаг перехода на летнее время: <0, 0, >0 */
};
```
Для преобразования человеко-читаемого времени в машинное используется функция `mktime`, а в обратную сторону - одной из функций: `gmtime` или `localtime`.



### Reentrant-функции

Многие функции POSIX API разрабатывались во времена однопроцессорных систем. Это может приводить к разным неприятным последствиям:

```
struct tm * tm_1 = localtime(NULL);
struct tm * tm_2 = localtime(NULL); // opps! *tm_1 changed!
```

Проблема заключается в том, что некоторые функции, например `localtime`, возвращает указатель на структуру-результат, а не скалярное значение. При этом, сами данные структуры не требуется удалять, - они хранятся в `.data`-области библиотеки glibc.

Проблема решается введением *повторно входимых (reentrant)* функций, которые в обязательном порядке трубуют в качестве одного из аргументов указатель на место в памяти для размещения результата:
```
struct tm tm_1; localtime_r(NULL, &tm_1);
struct tm tm_2; localtime_r(NULL, &tm_2); // OK
```

Использование повторно входимых функций является обязательным (но не достаточным) условием при написании многопоточных программ.


## FUSE

Файловые системы обычно реализуются в виде модулей ядра, которые работают в адресном пространстве ядра.

Монтирование осуществляется командой mount(8), которой необходимо указать:

* *точку монтирования* - каталог в виртуальной файловой системе, в котором будет доступно содержимое смонтированной файловой системы;
* *тип файловой системы* - один из поддерживаемых типов: ext2, vfat и др. Если не указать тип файловой системы, то ядро попытается автоматически определить её тип, но сделать это ему не всегда удаётся;
* *устройство для монтирования* - как правило, блочное, устройство для монтирования реальных устройств, либо URI для сетевых ресурсов, либо имя файла для монтирования образа.

**FUSE (Filesystem in Userspace)** - фреймворк для создания файловых систем в пользовательском пространстве. Для написания своей файловой системы надо будет переопределить функции read, write, stat и т.п.

Подробнее можно почитать в [ридинге Яковлева](https://github.com/victor-yacovlev/fpmi-caos/tree/master/practice/fuse).


```python

```
