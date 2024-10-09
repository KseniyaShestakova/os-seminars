
    .text
    .global get        // делаем функцию get глобальной
    
get:
    sub sp, sp, 16     // sp -= 16, аллоцируем 16 байт на стеке
    
    str lr, [sp, 8]    // сохраняем адсрес возврата по адресу sp+8
    
    adr x0, format_str // сохраняем адрес format_str в x0
    
    mov x1, sp         // x1 = sp - адрес int64_t для записи прочитанного в scanf

    bl scanf           // вызов scanf
    
    ldr x0, [sp]       // загружаем прочитанное значение в x0
    
    ldr lr, [sp, 8]    // восстанавливаем значение lr
    
    add sp, sp, 16     // освобождаем ранее выделенную память на стеке
    
    ret
    
    .section .rodata
format_str:
    .string "%ld"
