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
