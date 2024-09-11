# Intro. Linux, C, компиляция, gdb, strace

## Bash Cheat Sheet


```bash
%%bash
pwd # вывести название рабочей директории
mkdir new_dir # создание директории
ls # список файлов в текущей директории
ls -a # в том числе скрытых
cd new_dir # перейти в директорию new_dir
touch new_file # создание нового файла
echo 'Hello world' # вывод в консоль
echo 'Hello world' > new_file # вывод в файл
cat new_file # вывести содержимое файла
touch src.txt
touch dst.txt
cp src.txt dst.txt # скопировать содержимое src.txt в dst.txt
mv src.txt dst.txt # переместить содержимое src.txt в dst.txt
rm dst.txt # удалить файл
cd .. # переместиться в родительскую директорию
rm -r new_dir # удалить директорию
```

    /home/xxeniash/os-seminars/sem01-intro
    new_dir
    Untitled.ipynb
    .
    ..
    .ipynb_checkpoints
    new_dir
    Untitled.ipynb
    Hello world
    Hello world


Другие полезные команды:
* `grep` - поиск по регулярному выражению
например, `ls | grep .txt$` - поиск имен всех файлов текущей директории, кончающихся на .txt
* `head`/ `tail` - чтение из начала/конца файла 

Для работы с файлами удобно использовать текстовый редактор (`vim`, `nano`, ...) или IDE (`CLion`, `VSCode`). У текстовых редакторов есть отдельные сеты команд, с которыми стоит разобраться заранее, чтобы облегчить себе жизнь.

## Компиляция
Компилировать будем с помощью `gcc` (в случае кода на C) или `g++` (в случае кода на C++).


```bash
%%bash
gcc main.c -o main # компилируем main.c, результат пишем в main
./main
```

    Hello world!

**Сталии компиляции**:
* Препроцессинг
* Компиляция
* Ассемблирование
* Линковка

### Препроцессинг
На этом этапе раскрываются include-ы, define-ы и другие директивы.
На выходе получаем корректный файл на C (C++).


```python
%cat preproc.c
```

    #include "preproc.h"
    
    int main() {
        return foo();
    }



```python
%cat preproc.h
```

    int foo() {
        return 42;
    }



```bash
%%bash
gcc -E preproc.c -o preproc_done.c
cat preproc_done.c
```

    # 0 "preproc.c"
    # 0 "<built-in>"
    # 0 "<command-line>"
    # 1 "/usr/include/stdc-predef.h" 1 3 4
    # 0 "<command-line>" 2
    # 1 "preproc.c"
    # 1 "preproc.h" 1
    int foo() {
        return 42;
    }
    # 2 "preproc.c" 2
    
    int main() {
        return foo();
    }


### Компиляция
Преобразование исходного кода на C/C++ в ассемблерный код.


```bash
%%bash
gcc -S main.c -o main.S
cat main.S
```

    	.file	"main.c"
    	.text
    	.section	.rodata
    .LC0:
    	.string	"Hello world!"
    	.text
    	.globl	main
    	.type	main, @function
    main:
    .LFB0:
    	.cfi_startproc
    	endbr64
    	pushq	%rbp
    	.cfi_def_cfa_offset 16
    	.cfi_offset 6, -16
    	movq	%rsp, %rbp
    	.cfi_def_cfa_register 6
    	leaq	.LC0(%rip), %rax
    	movq	%rax, %rdi
    	movl	$0, %eax
    	call	printf@PLT
    	movl	$0, %eax
    	popq	%rbp
    	.cfi_def_cfa 7, 8
    	ret
    	.cfi_endproc
    .LFE0:
    	.size	main, .-main
    	.ident	"GCC: (Ubuntu 11.2.0-7ubuntu2) 11.2.0"
    	.section	.note.GNU-stack,"",@progbits
    	.section	.note.gnu.property,"a"
    	.align 8
    	.long	1f - 0f
    	.long	4f - 1f
    	.long	5
    0:
    	.string	"GNU"
    1:
    	.align 8
    	.long	0xc0000002
    	.long	3f - 2f
    2:
    	.long	0x3
    3:
    	.align 8
    4:


### Ассемблирование
Преобразование ассемблерного кода в машинный.


```bash
%%bash
gcc -c main.c -o main.o
```

### Линковка
Ее так же называют компановкой. Если компилируем одновременно несколько файлов, то на этом этапе происходит их сборка в один итоговый файл - исполняемый файл либо библиотеку. Линковка производится утилитой `ld`, которую автоматически вызывает `gcc`.

## Отладка

## GDB
`gdb` - GNU debugger.


```bash
%%bash
gcc -g main.c -o main # компилируем с опцией отладки -g
gdb main
```

    GNU gdb (Ubuntu 11.1-0ubuntu2) 11.1
    Copyright (C) 2021 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.
    Type "show copying" and "show warranty" for details.
    This GDB was configured as "x86_64-linux-gnu".
    Type "show configuration" for configuration details.
    For bug reporting instructions, please see:
    <https://www.gnu.org/software/gdb/bugs/>.
    Find the GDB manual and other documentation resources online at:
        <http://www.gnu.org/software/gdb/documentation/>.
    
    For help, type "help".
    Type "apropos word" to search for commands related to "word"...
    Reading symbols from main...
    (gdb) quit


Полезные команды (и один шорт кат):
* `Ctrl+X+A` - открыть окно с кодом
* `r` - начать исполнение команды
* `n` - перейти на следующую строку
* `b <function>` - создать брейкпоинт для остановки на функции function
* `c` - перейти на следующий брейкпоинт
* `s` - зайти внутрь функции
* `p <var>` - вывести значение переменной var
* `q` - закончить отладку

**Пример:** ловим segfault.


```bash
%%bash
cat segfault.c
```

    #include <stdio.h>
    
    int access(int* a, int i) { 
        return a[i]; 
    }
    
    int main() {
        int a[2] = {41, 42};
        int i = 100501;
        printf("a[%d] = %d\n", i, access(a, i)); // обращение к "чужой" памяти
    }



```bash
%%bash
gcc -g segfault.c -o segfault
gdb -ex=r -batch ./segfault
```

    [Thread debugging using libthread_db enabled]
    Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
    
    Program received signal SIGSEGV, Segmentation fault.
    0x000055555555518c in access (a=0x7fffffffdd10, i=100501) at segfault.c:4
    4	    return a[i]; 


## Sanitizers
Встраиваются в код программы и помогают ловить баги: AddressSanitizer, ThreadSanitizer, и другие. Разберем на примере AddressSanitizer.

### Segfault


```bash
%%bash
gcc -g -fsanitize=address segfault.c -o segfault # компилируем с санитайзером
./segfault
```

    AddressSanitizer:DEADLYSIGNAL
    =================================================================
    ==9146==ERROR: AddressSanitizer: SEGV on unknown address 0x7fff1880b604 (pc 0x5634433d52a8 bp 0x7fff187a9370 sp 0x7fff187a9360 T0)
    ==9146==The signal is caused by a READ memory access.
        #0 0x5634433d52a8 in access /home/xxeniash/os-seminars/sem01-intro/segfault.c:4
        #1 0x5634433d53c7 in main /home/xxeniash/os-seminars/sem01-intro/segfault.c:10
        #2 0x7f2d418c4fcf in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
        #3 0x7f2d418c507c in __libc_start_main_impl ../csu/libc-start.c:409
        #4 0x5634433d5184 in _start (/home/xxeniash/os-seminars/sem01-intro/segfault+0x1184)
    
    AddressSanitizer can not provide additional info.
    SUMMARY: AddressSanitizer: SEGV /home/xxeniash/os-seminars/sem01-intro/segfault.c:4 in access
    ==9146==ABORTING



    ---------------------------------------------------------------------------

    CalledProcessError                        Traceback (most recent call last)

    Input In [9], in <cell line: 1>()
    ----> 1 get_ipython().run_cell_magic('bash', '', 'gcc -g -fsanitize=address segfault.c -o segfault # компилируем с санитайзером\n./segfault\n')


    File ~/.local/lib/python3.9/site-packages/IPython/core/interactiveshell.py:2357, in InteractiveShell.run_cell_magic(self, magic_name, line, cell)
       2355 with self.builtin_trap:
       2356     args = (magic_arg_s, cell)
    -> 2357     result = fn(*args, **kwargs)
       2358 return result


    File ~/.local/lib/python3.9/site-packages/IPython/core/magics/script.py:153, in ScriptMagics._make_script_magic.<locals>.named_script_magic(line, cell)
        151 else:
        152     line = script
    --> 153 return self.shebang(line, cell)


    File ~/.local/lib/python3.9/site-packages/IPython/core/magics/script.py:305, in ScriptMagics.shebang(self, line, cell)
        300 if args.raise_error and p.returncode != 0:
        301     # If we get here and p.returncode is still None, we must have
        302     # killed it but not yet seen its return code. We don't wait for it,
        303     # in case it's stuck in uninterruptible sleep. -9 = SIGKILL
        304     rc = p.returncode or -9
    --> 305     raise CalledProcessError(rc, cell)


    CalledProcessError: Command 'b'gcc -g -fsanitize=address segfault.c -o segfault # \xd0\xba\xd0\xbe\xd0\xbc\xd0\xbf\xd0\xb8\xd0\xbb\xd0\xb8\xd1\x80\xd1\x83\xd0\xb5\xd0\xbc \xd1\x81 \xd1\x81\xd0\xb0\xd0\xbd\xd0\xb8\xd1\x82\xd0\xb0\xd0\xb9\xd0\xb7\xd0\xb5\xd1\x80\xd0\xbe\xd0\xbc\n./segfault\n'' returned non-zero exit status 1.


### Утечка памяти


```bash
%%bash
cat memory_leak.c
```

    #include <stdio.h>
    #include <stdlib.h>
    
    int main() {
        malloc(8);
    }



```bash
%%bash
gcc -g -fsanitize=address memory_leak.c -o memory_leak
./memory_leak
```

    memory_leak.c: In function ‘main’:
    memory_leak.c:5:5: warning: ignoring return value of ‘malloc’ declared with attribute ‘warn_unused_result’ [-Wunused-result]
        5 |     malloc(8);
          |     ^~~~~~~~~
    
    =================================================================
    ==9309==ERROR: LeakSanitizer: detected memory leaks
    
    Direct leak of 8 byte(s) in 1 object(s) allocated from:
        #0 0x7f82fc65b867 in __interceptor_malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:145
        #1 0x563d5028419a in main /home/xxeniash/os-seminars/sem01-intro/memory_leak.c:5
        #2 0x7f82fc3a8fcf in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    
    SUMMARY: AddressSanitizer: 8 byte(s) leaked in 1 allocation(s).



    ---------------------------------------------------------------------------

    CalledProcessError                        Traceback (most recent call last)

    Input In [11], in <cell line: 1>()
    ----> 1 get_ipython().run_cell_magic('bash', '', 'gcc -g -fsanitize=address memory_leak.c -o memory_leak\n./memory_leak\n')


    File ~/.local/lib/python3.9/site-packages/IPython/core/interactiveshell.py:2357, in InteractiveShell.run_cell_magic(self, magic_name, line, cell)
       2355 with self.builtin_trap:
       2356     args = (magic_arg_s, cell)
    -> 2357     result = fn(*args, **kwargs)
       2358 return result


    File ~/.local/lib/python3.9/site-packages/IPython/core/magics/script.py:153, in ScriptMagics._make_script_magic.<locals>.named_script_magic(line, cell)
        151 else:
        152     line = script
    --> 153 return self.shebang(line, cell)


    File ~/.local/lib/python3.9/site-packages/IPython/core/magics/script.py:305, in ScriptMagics.shebang(self, line, cell)
        300 if args.raise_error and p.returncode != 0:
        301     # If we get here and p.returncode is still None, we must have
        302     # killed it but not yet seen its return code. We don't wait for it,
        303     # in case it's stuck in uninterruptible sleep. -9 = SIGKILL
        304     rc = p.returncode or -9
    --> 305     raise CalledProcessError(rc, cell)


    CalledProcessError: Command 'b'gcc -g -fsanitize=address memory_leak.c -o memory_leak\n./memory_leak\n'' returned non-zero exit status 1.


## Strace
Отладка системных вызовов


```bash
%%bash
cat print.c
```

    #include <stdio.h>
    
    int main() {
        printf("Hello world!\n");
    }



```bash
%%bash
gcc print.c -o print
strace ./print
```

    execve("./print", ["./print"], 0x7fff2746acb0 /* 62 vars */) = 0
    brk(NULL)                               = 0x5561547a8000
    arch_prctl(0x3001 /* ARCH_??? */, 0x7ffe843c86c0) = -1 EINVAL (Invalid argument)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
    newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=121691, ...}, AT_EMPTY_PATH) = 0
    mmap(NULL, 121691, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f84467e9000
    close(3)                                = 0
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0000\242\2\0\0\0\0\0"..., 832) = 832
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    pread64(3, "\4\0\0\0 \0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0"..., 48, 848) = 48
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\360\374)\26\\\276`\210\300\341\255\360;\0H\373"..., 68, 896) = 68
    newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=2216272, ...}, AT_EMPTY_PATH) = 0
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f84467e7000
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    mmap(NULL, 2260560, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f84465bf000
    mprotect(0x7f84465e7000, 2019328, PROT_NONE) = 0
    mmap(0x7f84465e7000, 1654784, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x28000) = 0x7f84465e7000
    mmap(0x7f844677b000, 360448, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1bc000) = 0x7f844677b000
    mmap(0x7f84467d4000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x214000) = 0x7f84467d4000
    mmap(0x7f84467da000, 52816, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f84467da000
    close(3)                                = 0
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f84465bd000
    arch_prctl(ARCH_SET_FS, 0x7f84467e85c0) = 0
    set_tid_address(0x7f84467e8890)         = 9420
    set_robust_list(0x7f84467e88a0, 24)     = 0
    mprotect(0x7f84467d4000, 16384, PROT_READ) = 0
    mprotect(0x556154405000, 4096, PROT_READ) = 0
    mprotect(0x7f844683a000, 8192, PROT_READ) = 0
    prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
    munmap(0x7f84467e9000, 121691)          = 0
    newfstatat(1, "", {st_mode=S_IFIFO|0600, st_size=0, ...}, AT_EMPTY_PATH) = 0
    getrandom("\x80\x2d\xc9\x34\xfd\x1e\x4a\x38", 8, GRND_NONBLOCK) = 8
    brk(NULL)                               = 0x5561547a8000
    brk(0x5561547c9000)                     = 0x5561547c9000
    write(1, "Hello world!\n", 13)          = 13
    exit_group(0)                           = ?
    +++ exited with 0 +++


    Hello world!



```python

```
