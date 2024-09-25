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
