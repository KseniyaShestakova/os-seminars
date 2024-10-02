# Assembler x86

**Два синтаксиса!** Исторически сложилось два синтаксиса языка ассемблера x86: синтаксис AT&T,
используемый в UNIX-системах, и синтаксис Intel, используемый в DOS/Windows.

Различие, в первую очередь, относится к порядку аргументов команд.

Компилятор gcc по умолчанию использует синтаксис AT&T, но с указанием опции
`-masm=intel` может переключаться в синтаксис Intel.

Кроме того, можно указать используемый синтаксис первой строкой в тексте
самой программы:
```nasm
.intel_syntax noprefix
```

Здесь параметр `noprefix` после `.intel_syntax` указывает на то, что помимо порядка аргументов, соответствующих синтаксису Intel, ещё и имена регистров не должны начинаться с символа `%`, а константы - с символа `$`, как это принято в синтаксисе AT&T.

Мы будем использовать именно этот синтаксис, поскольку с его использованием
написано большинство доступной документации и примеров, включая документацию
от производителей процессоров.

## Регистры

**Регистр** - быстрая ячейка памяти. Ее размер зависит от битности архитектуры.


**Историческая справка** 

| Год  | Регистры           | Битность | Первый процессор | Комментарий |
|------|--------------------|----------|------------------|-------------|
| 1974 | a, b, c, d         | 8 bit    | Intel 8080       | |
| 1978 | ax, bx, cx, dx     | 16 bit   | Intel 8086       | X - eXtended ([совсем ненадежный источник](https://stackoverflow.com/a/892948))|
| 1985 | eax, ebx, exc, edx | 32 bit   | Intel 80386      | E - extended |
| 2003 | rax, rbx, rcx, rdx | 64 bit   | AMD Opteron      | R - (внезапно) register |


Как оно выглядит сейчас в 64-битных процессорах

<table width="800px" border="1" style="text-align:center; font-family: Courier New; font-size: 10pt">

<tbody><tr>
<td colspan="8" width="25%" style="background:lightgrey">RAX

<td colspan="8" width="25%" style="background:lightgrey">RCX

<td colspan="8" width="25%" style="background:lightgrey">RDX

<td colspan="8" width="25%" style="background:lightgrey">RBX

<tr>
<td colspan="4" width="12.5%">
<td colspan="4" width="12.5%" style="background:lightgrey">EAX

<td colspan="4" width="12.5%">
<td colspan="4" width="12.5%" style="background:lightgrey">ECX

<td colspan="4" width="12.5%">
<td colspan="4" width="12.5%" style="background:lightgrey">EDX

<td colspan="4" width="12.5%">
<td colspan="4" width="12.5%" style="background:lightgrey">EBX

<tr>
<td colspan="6" width="18.75%">
<td colspan="2" width="6.25%" style="background:lightgrey">AX

<td colspan="6" width="18.75%">
<td colspan="2" width="6.25%" style="background:lightgrey">CX

<td colspan="6" width="18.75%">
<td colspan="2" width="6.25%" style="background:lightgrey">DX

<td colspan="6" width="18.75%">
<td colspan="2" width="6.25%" style="background:lightgrey">BX

<tr>
<td colspan="6" width="18.75%">
<td width="3.125%" style="background:lightgrey">AH
<td width="3.125%" style="background:lightgrey">AL

<td colspan="6" width="18.75%">
<td width="3.125%" style="background:lightgrey">CH
<td width="3.125%" style="background:lightgrey">CL

<td colspan="6" width="18.75%">
<td width="3.125%" style="background:lightgrey">DH
<td width="3.125%" style="background:lightgrey">DL

<td colspan="6" width="18.75%">
<td width="3.125%" style="background:lightgrey">BH
<td width="3.125%" style="background:lightgrey">BL
</tbody></table>

    
Сейчас в x86-64 есть 16 регистров общего назначения и 8 больших регистров для операций над числами с плавающей точкой.
    

Регистры x86:
* RAX - Accumulator Register
* RBX - Base Register
* RCX - Counter Register
* RDX - Data Register
* RSI - Source Index
* RDI - Destination Index
* RBP - Base Pointer
* RSP - Stack Pointer
* R8...R15 - дополнительные регистры общего назначения

Регистры в x86-64:
* `rax`, `rbx`, `rcx`, `rdx` - регистры общего назначения.
* `rsp` - указатель на вершину стека
* `rbp` - указатель на начало стекового фрейма (но можно использовать аккуратно использовать как регистр общего назначения)
* `rsi`, `rdi` - странные регистры для копирования массива, по сути регистры общего назначения, но ограниченные в возможностях.
* `r8`...`r15`- дополнительные регистры
  
## Соглашения о вызовах (Calling Conventions)
* Целочисленные аргументы передаются последовательно в регистрах `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`. Если этого не хватило, то остальные аргументы передаются последовательно через стек
* Вещественные аргументы передаются через регистры `xmm0`, ..., `xmm7`
* Возвращаемое значение функции записывается в `rax`
* Вызываемая функция обязана сохранить на стеке значения регистров общего назначения `rbx`, `rbp`, `r12`, ..., `r15`
* Перед вызовом функции (в 64-разрядной архитектуре) стек должен быть вырвнен по границе 16 байт, т.е. необходимо уменьшить значение `rsp` таким образом, чтобы оно было кратно 16. Если стек задействуется для передачи параметров, то они должны быть прижаты к нижней выровненной границе стека

Для функций гарантируется 128-байтная "красная зона" на стеке ниже регистра `rsp` - область, которая не будет затронута внешним событием, например, обработчиком сигнала. Эту область можно задействовать для адресации локальных переменных.
    
## Инструкции x86
[справка](https://www.felixcloutier.com/x86/)
    
### Арифметические операции
```nasm
add     DST, SRC        /* DST += SRC */
sub     DST, SRC        /* DST -= SRC */
inc     DST             /* ++DST */
dec     DST             /* --DST */
neg     DST             /* DST = -DST */
mov     DST, SRC        /* DST = SRC */
imul    SRC             /* eax = eax * SRC - знаковое */
mul     SRC             /* eax = eax * SRC - беззнаковое */
and     DST, SRC        /* DST &= SRC */
or      DST, SRC        /* DST |= SRC */
xor     DST, SRC        /* DST ^= SRC */
not     DST             /* DST = ~DST */
cmp     DST, SRC        /* DST - SRC, результат не сохраняется, */
test    DST, SRC        /* DST & SRC, результат не сохраняется  */
adc     DST, SRC        /* DST += SRC + CF */
sbb     DST, SRC        /* DST -= SRC - CF */
```

Для AT&T порядок аргументов будет противоположным.

### Флаги процессора
Большинство инструкций в синтаксисе Intel выставляют флаги.

Флаги:
* `ZF` - в результате операции был получен ноль
* `SF` - в результате операции было получено отрицательное число
* `CF` - в результате операции произошел перенос старшего бита результата
* `OF` - в результате операции произошло переполнение знакового результата

`test` и `cmp` не сохраняют результат, а только меняют флаги.

### Управнение ходом программы
Безусловный переход выполняется с помощью инструкции `jmp`
```nasm
jmp label
```

Условные переходы проверяют комбинации арифметических флагов:
```nasm
jz      label   /* переход, если равно (нуль), ZF == 1 */
jnz     label   /* переход, если не равно (не нуль), ZF == 0 */
jc      label   /* переход, если CF == 1 */
jnc     label   /* переход, если CF == 0 */
jo      label   /* переход, если OF == 1 */
jno     label   /* переход, если OF == 0 */
jg      label   /* переход, если больше для знаковых чисел */
jge     label   /* переход, если >= для знаковых чисел */
jl      label   /* переход, если < для знаковых чисел */
jle     label   /* переход, если <= для знаковых чисел */
ja      label   /* переход, если > для беззнаковых чисел */
jae     label   /* переход, если >= (беззнаковый) */
jb      label   /* переход, если < (беззнаковый) */
jbe     label   /* переход, если <= (беззнаковый) */
```
Вызов функции и возврат из неё осуществляются командами `call` и `ret`
```nasm
call    label   /* складывает в стек адрес возврата, и переход на label */
ret             /* вытаскивает из стека адрес возврата и переходит к нему */
```

Кроме того, есть составная команда для организации циклов, которая
подразумевает, что в регистре `ecx` находится счётчик цикла:
```nasm
loop    label   /* уменьшает значение ecx на 1; если ecx==0, то
                   переход на следующую инструкцию, в противном случае
                   переход на label */
```
    

## Компиляция и запуск
Дисассемблирование кода на С:
```
gcc -m64 -masm=intel -S -O3 code.c -o code.S
```
Компиляция кода на ассемблере:
```
gcc -m64 -masm=intel disasom_example.S -o run
```
Запуск (как обычно):
```
./run
```

[Compiler explorer](https://godbolt.org/)

### Пример дисасемблирования


```bash
%%bash
cat long_sum.c
```

    #include <stdint.h>
    
    int64_t sum(int32_t a, int32_t b, int32_t c) {
        return a + b + c;
    }



```bash
%%bash
gcc -m64 -masm=intel -O3 long_sum.c -S -o long_sum.S
cat long_sum.S
```

    	.file	"long_sum.c"
    	.intel_syntax noprefix
    	.text
    	.p2align 4
    	.globl	sum
    	.type	sum, @function
    sum:
    .LFB0:
    	.cfi_startproc
    	endbr64
    	add	edi, esi
    	add	edi, edx
    	movsx	rax, edi
    	ret
    	.cfi_endproc
    .LFE0:
    	.size	sum, .-sum
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


Оставим только важное, тогда получится:
```asm
sum:
    endbr64
	add	edi, esi
	add	edi, edx
	movsx	rax, edi
	ret
```

`endbr64` - [Введение в аппаратную защиту стека / Хабр](https://habr.com/ru/post/494000/) и [control-flow-enforcement-technology](https://software.intel.com/sites/default/files/managed/4d/2a/control-flow-enforcement-technology-preview.pdf)


`movsx` - перемещение данных с расширением разрядности (`sx` - sign extension)

## Пример с вызовом функций
Напишем код на С, посмотрим на его дисассемблер, а потом напишем свой код на ассемблере.


```bash
%%bash
cat call_example.c
```

    #include <inttypes.h>
    #include <stdio.h>
    
    extern int *A;
    
    void cyclic_sum(size_t N) {
        for (size_t i = 0; i < N; ++i) {
            int32_t num = 0;
            scanf("%d", &num);
            int64_t result = num;
            result *= A[i];
            printf("%d\n", result);
        }    
    }



```bash
%%bash
gcc -m64 -masm=intel -O3 call_example.c -S -o call_example.S
cat call_example.S
```

    call_example.c: In function ‘cyclic_sum’:
    call_example.c:12:18: warning: format ‘%d’ expects argument of type ‘int’, but argument 2 has type ‘int64_t’ {aka ‘long int’} [-Wformat=]
       12 |         printf("%d\n", result);
          |                 ~^     ~~~~~~
          |                  |     |
          |                  int   int64_t {aka long int}
          |                 %ld
    call_example.c:9:9: warning: ignoring return value of ‘scanf’ declared with attribute ‘warn_unused_result’ [-Wunused-result]
        9 |         scanf("%d", &num);
          |         ^~~~~~~~~~~~~~~~~


    	.file	"call_example.c"
    	.intel_syntax noprefix
    	.text
    	.section	.rodata.str1.1,"aMS",@progbits,1
    .LC0:
    	.string	"%d"
    .LC1:
    	.string	"%d\n"
    	.text
    	.p2align 4
    	.globl	cyclic_sum
    	.type	cyclic_sum, @function
    cyclic_sum:
    .LFB23:
    	.cfi_startproc
    	endbr64
    	push	r14
    	.cfi_def_cfa_offset 16
    	.cfi_offset 14, -16
    	push	r13
    	.cfi_def_cfa_offset 24
    	.cfi_offset 13, -24
    	push	r12
    	.cfi_def_cfa_offset 32
    	.cfi_offset 12, -32
    	push	rbp
    	.cfi_def_cfa_offset 40
    	.cfi_offset 6, -40
    	push	rbx
    	.cfi_def_cfa_offset 48
    	.cfi_offset 3, -48
    	sub	rsp, 16
    	.cfi_def_cfa_offset 64
    	mov	rax, QWORD PTR fs:40
    	mov	QWORD PTR 8[rsp], rax
    	xor	eax, eax
    	test	rdi, rdi
    	je	.L1
    	mov	rbp, rdi
    	xor	ebx, ebx
    	lea	r14, 4[rsp]
    	lea	r13, .LC0[rip]
    	lea	r12, .LC1[rip]
    	.p2align 4,,10
    	.p2align 3
    .L3:
    	mov	rsi, r14
    	mov	rdi, r13
    	xor	eax, eax
    	mov	DWORD PTR 4[rsp], 0
    	call	__isoc99_scanf@PLT
    	mov	rdx, QWORD PTR A[rip]
    	movsx	rax, DWORD PTR 4[rsp]
    	mov	rsi, r12
    	mov	edi, 1
    	movsx	rdx, DWORD PTR [rdx+rbx*4]
    	add	rbx, 1
    	imul	rdx, rax
    	xor	eax, eax
    	call	__printf_chk@PLT
    	cmp	rbp, rbx
    	jne	.L3
    .L1:
    	mov	rax, QWORD PTR 8[rsp]
    	sub	rax, QWORD PTR fs:40
    	jne	.L11
    	add	rsp, 16
    	.cfi_remember_state
    	.cfi_def_cfa_offset 48
    	pop	rbx
    	.cfi_def_cfa_offset 40
    	pop	rbp
    	.cfi_def_cfa_offset 32
    	pop	r12
    	.cfi_def_cfa_offset 24
    	pop	r13
    	.cfi_def_cfa_offset 16
    	pop	r14
    	.cfi_def_cfa_offset 8
    	ret
    .L11:
    	.cfi_restore_state
    	call	__stack_chk_fail@PLT
    	.cfi_endproc
    .LFE23:
    	.size	cyclic_sum, .-cyclic_sum
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



```bash
%%bash
cat call_example_hand_made.S
```

    	.intel_syntax noprefix
    
    	.text                                  
    	.globl very_important_function 
    
    cyclic_sum:
    
    	push   r12  
    	mov    r12, 0                    //  i == r12 = 0 
    
        push   r13 
    	mov    r13, rdi			         //  r13 = N
    
    	sub    rsp, 8                    //  allocate 8 bytes on stack
    
    .loop_begin:
    	cmp    r12, r13                  //  if (i >= N) 
    	jae    .loop_end                 //  break
    
    	lea    rdi, .fmt_scanf[rip]      //  load into `rdi` char* "%d"
        mov    rsi, rsp                  //  load into `rsi` pointer to int32_t on stack
    
        call   scanf                     //  call scanf
    
    	movsxd rsi, DWORD PTR [rsp]      //  load into `rsi` int32_t value from stack
        mov    rcx, A[rip]               //  load into `rcx` address of A
    
    	movsxd rcx, [rcx + 4 * r12]      //  load into `rcx` *(A + i * 4) - to multiply rsi and A[i]
        imul   rsi, rcx                  //  store in `rax` value of (rsi * rcx)
    
    	lea    rdi, .fmt_printf[rip]     //  load into `rdi` char* "%lld\n"
    
        call   printf                    // call printf
    
        inc    r12                       // increment `r12` i. e. `++i`
        jmp    .loop_begin     
    
    
    .loop_end:
    	add    rsp, 8                    // free allocated stack memory
    
        pop    r13                       // restore saved on stack value of `r13` 
        pop    r12                       // restore saved on stack value of `r12`
    
    	ret
    
    	.section .rodata
    .fmt_scanf:
        .string "%d"
    .fmt_printf:
        .string "%lld\n"


## Ассемблерные вставки


```bash
%%bash
cat assembler_inside_c.c
```

    
    #include <stdio.h>
    
    int strange_function_L4(int a, int b, int c);
    __asm__ (R"(
    .global strange_function_L4
    strange_function_L4:
        endbr64
        add    edi, esi
        add    edi, edx
        movsx    rax, edi
        ret
    )");
    
    int main() {
        printf("%d\n", strange_function_L4(4, 0, 1));
        printf("%d\n", strange_function_L4(4, 2, 2));
        printf("%d\n", strange_function_L4(5, 0, 3));
        printf("%d\n", strange_function_L4(5, 2, 4));
        return 0;
    }



```bash
%%bash
gcc -masm=intel assembler_inside_c.c -o assembler_inside_c
./assembler_inside_c
```

    5
    8
    8
    11



```python

```
