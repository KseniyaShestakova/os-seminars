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
