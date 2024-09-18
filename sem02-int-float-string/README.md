# Представление целых и вещественных чисел и строк

##  Целочисленная арифметика
Для операций сложения, вычитания, умножения знаковость числа не важна. Для $<, >, /$ важна. Сравнение знакового и беззнакового числа - UB!
### Беззнаковые числа
Если рассматриваем $n$-битные беззнаковые целые числа, то операция сложения над ними аналогична операции сложения в группе $\mathbb{Z}_{2^n}$. Ситуация, когда результат сложения (в смысле $\mathbb{Z})$ не помещается в $n$ бит, называется *переполнением*.

### Знаковые числа
Переполнение для знаковых чисел - UB! Компилятор по умолчанию считает, что переполнения знаковых чисел не произойдет, и может использовать это для оптимизаций.


```bash
%%bash
cat overflow.c
```

    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    
    bool check_inc(int32_t x) {
        return x < x + 1;
    }
    
    bool check_uinc(uint32_t x) {
        return x < x + 1;
    }
    
    
    int main() {
        int32_t signed_int = INT32_MAX;
        printf("Signed check for %d: %d\n", signed_int, check_inc(signed_int));
        printf("Increment for %d: %d\n", signed_int, signed_int + 1);
    
        uint32_t unsigned_int = UINT32_MAX;
        printf("Unsigned check for %u: %u\n", unsigned_int, check_uinc(unsigned_int));
        printf("Increment for %u: %u\n", unsigned_int, unsigned_int + 1);
        
    }


Посмотрим, во что превращаются функции `check_inc` и `check_uinc` при компиляции.


```bash
%%bash
gcc -O3 -g overflow.c -o overflow
gdb overflow -batch -ex="disass check_inc" -ex="disass check_uinc"
```

    Dump of assembler code for function check_inc:
       0x00000000000011d0 <+0>:	endbr64 
       0x00000000000011d4 <+4>:	mov    $0x1,%eax
       0x00000000000011d9 <+9>:	ret    
    End of assembler dump.
    Dump of assembler code for function check_uinc:
       0x00000000000011e0 <+0>:	endbr64 
       0x00000000000011e4 <+4>:	cmp    $0xffffffff,%edi
       0x00000000000011e7 <+7>:	setne  %al
       0x00000000000011ea <+10>:	ret    
    End of assembler dump.


Словить переполнение можно с помощью санитайзера.


```bash
%%bash
gcc -O0 -g -fsanitize=undefined overflow.c -o overflow
./overflow
```

    overflow.c:17:5: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'


    Signed check for 2147483647: 1
    Increment for 2147483647: -2147483648
    Unsigned check for 4294967295: 0
    Increment for 4294967295: 0


Для обработки переполнений можно использовать, например, насыщением по правилу: $a + b = \min(a + b, MAX)$. Можно сделать с помощью встроенных функций, можно реализовать самим.


```python
%%cpp

#include <assert.h>
#include <stdint.h>

unsigned int sat_add(unsigned int x, unsigned int y) {
    unsigned int z;
    // Функция, которая обрабатывает выставленный процессором флаг и возвращает его явно
    if (__builtin_uadd_overflow(x, y, &z)) {
        return -1;
    }
    return z;
}

int sat_add_handmade(int a, int b) {
    if (b > 0) {
        return (a < INT32_MAX) ? a + b : INT32_MAX;
    }
    if (a > 0) return a + b;
    return (INT32_MIN - a < b) ? a + b : INT32_MIN;
}
```

    UsageError: Cell magic `%%cpp` not found.



```bash
%%bash
cat int_operations.c
```

    typedef unsigned int uint;
    
    int sum(int x, int y) { return x + y; }
    uint usum(uint x, uint y) { return x + y; }
    
    int mul(int x, int y) { return x * y; }
    uint umul(uint x, uint y) { return x * y; }
    
    int cmp(int x, int y) { return x < y; }
    int ucmp(uint x, uint y) { return x < y; }
    
    int div(int x, int y) { return x / y; }
    int udiv(uint x, uint y) { return x / y; }
    
    int main() {}



```bash
%%bash
gcc -O0 -g int_operations.c -o int_operations -Os -Wl,--gc-sections -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr
gdb int_operations -batch -ex="disass sum" -ex="disass usum"
gdb int_operations -batch -ex="disass div" -ex="disass udiv"
gdb int_operations -batch -ex="disass cmp" -ex="disass ucmp"
```

    Dump of assembler code for function sum:
       0x0000000000001139 <+0>:	lea    (%rdi,%rsi,1),%eax
       0x000000000000113c <+3>:	ret    
    End of assembler dump.
    Dump of assembler code for function usum:
       0x000000000000113d <+0>:	lea    (%rdi,%rsi,1),%eax
       0x0000000000001140 <+3>:	ret    
    End of assembler dump.
    Dump of assembler code for function div:
       0x000000000000115d <+0>:	mov    %edi,%eax
       0x000000000000115f <+2>:	cltd   
       0x0000000000001160 <+3>:	idiv   %esi
       0x0000000000001162 <+5>:	ret    
    End of assembler dump.
    Dump of assembler code for function udiv:
       0x0000000000001163 <+0>:	mov    %edi,%eax
       0x0000000000001165 <+2>:	xor    %edx,%edx
       0x0000000000001167 <+4>:	div    %esi
       0x0000000000001169 <+6>:	ret    
    End of assembler dump.
    Dump of assembler code for function cmp:
       0x000000000000114d <+0>:	xor    %eax,%eax
       0x000000000000114f <+2>:	cmp    %esi,%edi
       0x0000000000001151 <+4>:	setl   %al
       0x0000000000001154 <+7>:	ret    
    End of assembler dump.
    Dump of assembler code for function ucmp:
       0x0000000000001155 <+0>:	xor    %eax,%eax
       0x0000000000001157 <+2>:	cmp    %esi,%edi
       0x0000000000001159 <+4>:	setb   %al
       0x000000000000115c <+7>:	ret    
    End of assembler dump.


Различие `cmp` и `ucmp` - в устанавливаемых флагах.

### Про размер целочисленных типов

Согласно стандарту языка C:

***

#### &nbsp; &nbsp; &nbsp;_тип_ &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;&nbsp; &nbsp; &nbsp; &nbsp; _размер_
    
- __(unsigned) char__ &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;$\ge$ **__8__** бит, обычно **__8__** 
    

- __(unsigned) short__ &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;&nbsp;&nbsp; &nbsp;$\ge$ **__16__** бит
    

- __(unsigned) int__ &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp; &nbsp; $\ge$ **__16__** бит, обычно **__32__**

    
- __(unsigned) long__ &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; $\ge$ **__32__** бит


- __(unsigned) long long__ &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; $\ge$ **__64__** бит, обычно **__64__**
    
   
 </font> 
    
Знаковость типа `char` не определена, для ее определения существуют опции компилятора `-fsigned-char` и `-funsigned-char`.

Не стоит полагаться на размерность целочисленного типа, лучше явно проверять ее с помощью оператора `sizeof`.

Для большей определенности стоит пользоваться типами с фиксированной разрядностью: `int32_t`, `uint32_t`, `int64_t` и т.д. Для того, чтобы использовать их, надо прописать include-ы: 
```
#include <stdint.h>
#include <inttypes.h>
```

### Битовые операции
* `^` - XOR
* `|` - OR
* `&` - AND
* `~` - отрицание
* `>>` - битовый сдвиг вправо (деление на два)
* `<<` - битовый сдвиг влево (умножение на два для беззнаковых чисел)

Для отрицательных чисел поведение битовых операторов не определено.

Немного задачек:
1. Получить $i$-ый бит числа $x$
2. Заменить $i$-ый бит числа $x$ на $b$
3. Занулить $i$-ый бит числа $x$
4. Инвертировать $i$-ый бит числа $x$
5. Получить биты числа $x$ с $i$-ого по $j$-ый невключительно как беззнаковое число
6. Скопировать в биты числа $x$ с $i$-ого по $j$-ый невключительно младшие биты числа $y$

Здесь $x,\ y$ - беззнаковые числа, $b$ - некоторый бит.

## Вещественные типы
Вещественные типы - это `float` и `double`. Значение высчитывается по формуле $(-1)^S \times M \times 2^{E - b} $, где $S$ - знак (sign), $M$ - мантисса (mantissa), $E$ - экспонента (exponenta), $b$ - смещение экспоненты (bias)
* `float`: $|S| = 1,\ |M| = 23,\ |E| = 8$
* `double`: $|S| = 1,\ |M| = 52,\ |E| = 11$


Классификация значений по стандарту [IEEE 754](http://www.softelectro.ru/ieee754.html):

|_S_|[|_Exp_|]|[|_Mantissa_|]|<center>_Type_</center>|
|-|-|---|-|-|------|-|---------------------|
|0|0|.......|0|0|... ... ... ... ...|0|<center>__PlusZero__</center>|
|1|0|.......|0|0|... ... ... ... ...|0|<center>__MinusZero__</center>|
|0|0|.......|0|x|... ... ... ... ...|x|<center>__PlusDenormalized__</center>|
|1|0|.......|0|x|... ... ... ... ...|x|<center>__MinusDenormalized__</center>|
|0|1|.......|1|0|... ... ... ... ...|0|<center>__PlusInf__</center>|
|1|1|.......|1|0|... ... ... ... ...|0|<center>__MinusInf__</center>|
|x|1|.......|1|1|x. ... ... ... .x|x|<center>__QuietNaN__</center>|
|x|1|.......|1|0|x. ... ... ... .x|x|<center>__SignalingNaN__</center>|

Немного о мотивации названий: если в ходе вычислений получается SignalingNaN, то будет выслан сигнал, в случае с QuietNaN программа продолжает работать. Например, SignalingNaN может получиться при делении на 0.

Значения, которые не подходят ни под одну из строк этой таблицы, называются регулярными.

## Как получить битовое представление числа?


```bash
%%bash
cat bit_cast.c
gcc bit_cast.c -o bit_cast
echo '--------------------------------------------------------'
./bit_cast
```

    #include <stdio.h>
    #include <inttypes.h>
    #include <stdint.h>
    #include <limits.h>
    
    void output_int(int64_t a_int) {
        for (size_t i = sizeof(a_int) * CHAR_BIT; i > 0; --i) {
            printf("%ld", (a_int & ((int64_t)1 << (i-1))) >> (i - 1));
         }
         printf("\n");
    
    }
    
    int64_t ptr_cast(double a) {
        double* a_ptr_double = &a;
        int64_t* a_ptr_int = (int64_t*)a_ptr_double;
     
        int64_t a_int = *a_ptr_int;
        return a_int;
    }
    
    typedef union {
        double double_value;
        uint64_t uint_value;
    } double_or_uint;
    
    
    int64_t union_cast(double a) {
        double_or_uint u;
    
        u.double_value = a;
    
        return u.uint_value;
    }
    
    
    
    int main() {
        double a = 0.01;
    
        printf("Bit presentation of %lf, ptr_cast: \n", a);
        output_int(ptr_cast(a));
    
        printf("Bit presentation of %lf, union_cast: \n", a);
        output_int(union_cast(a));
    
        return 0;
    }
    --------------------------------------------------------
    Bit presentation of 0.010000, ptr_cast: 
    0011111110000100011110101110000101000111101011100001010001111011
    Bit presentation of 0.010000, union_cast: 
    0011111110000100011110101110000101000111101011100001010001111011


## Кодировки 
Терминология:
* Character — что-то, что мы хотим представить
* Character set — какое-то множество символов
* Coded character set (CCS) — отображение символов в уникальные номера
* Code point — уникальный номер какого-то символа

---

### ASCII
* American Standard Code for Information Interchange, 1963 год
* 7-ми битная кодировка, то есть кодирует 128 различных символов
* Control characters: c 0 по 31 включительно, непечатные символы, мета-информация для терминалов


### Unicode
* Codespace: 0 до 0x10FFFF (~1.1 млн. code points)
* Code point'ы обозначаются как U+<число>
* `ℵ` = U+2135
* `r` = U+0072
* Unicode — не кодировка: он не определяет как набор байт трактовать как characters. Кодировка - это отображение байт в битовое представление, она задается UTF-32, UTF-8


### UTF-32
* Использует всегда 32 бита (4 байта) для кодировки
* Используется во внутреннем представлении строк в некоторых языках (например, Python)
* Позволяет обращаться к произвольному code point'у строки за O(1)
* BOM определяет little vs big endian


### UTF-8
* Unicode Transformation Format
* Определяет способ как будут преобразовываться code point'ы
* Переменная длина: от 1 байта (ASCII) до 4 байт

---
### UTF-8
![center width:1000px](./utf8.svg)


```python

```
