#include <stdio.h>
#include <dirent.h>
#include <assert.h>
#include <fnmatch.h>

int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    DIR *pDir = opendir(dir_path);
    if (pDir == NULL) {
        fprintf(stderr, "Cannot open directory '%s'\n", dir_path);
        return 1;
    }

    int limit = 10;
    for (struct dirent *pDirent; (pDirent = readdir(pDir)) != NULL && limit > 0; --limit) {
            printf("%s\n", pDirent->d_name);
        
    }
    closedir(pDir);
    return 0;

}
