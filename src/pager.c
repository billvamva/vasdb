#include "pager.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

Pager* OpenPager(const char* fileName)
{
    int fd = open(fileName,
        O_RDWR | // Read/Write mode
            O_CREAT, // Create file if it does not exist
        S_IWUSR | // User write permission
            S_IRUSR // User read permission
    );

    if (fd == -1) {
        printf("unable to open file\n");
        exit(EXIT_SUCCESS);
    }

    off_t fileLength = lseek(fd, 0, SEEK_END);
    Pager* pager = (Pager*)malloc(sizeof(Pager));
    pager->fileDescriptor = fd;
    pager->fileLength = fileLength;

    for (int i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}
