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
