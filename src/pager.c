#include "pager.h"
#include "sqlcompiler.h"
#include <fcntl.h>
#include <stdint.h>
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

void* GetPage(Pager* pager, uint32_t pageNum)
{
    if (pageNum > TABLE_MAX_PAGES) {
        printf("trying to retrieve page that exceeds limit, %d > %d", pageNum, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    // cache miss
    if (pager->pages[pageNum] == NULL) {
        void* page = malloc(PAGE_SIZE);
        uint32_t numPages = pager->fileLength / PAGE_SIZE;

        if (pager->fileLength % PAGE_SIZE) {
            numPages++;
        }

        if (pageNum <= numPages) {
            lseek(pager->fileDescriptor, pageNum * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->fileDescriptor, page, PAGE_SIZE);
            if (bytes_read == -1) {
                printf("could not read db file\n");
                exit(EXIT_FAILURE);
            }
        }

        pager->pages[pageNum] = page;
    }

    return pager->pages[pageNum];
}

void FlushPager(Pager* pager, uint32_t pageNum, uint32_t size)
{
    if (pager->pages[pageNum] == NULL) {
        printf("trying to flush null page \n");
        exit(EXIT_FAILURE);
    }
    off_t offset = lseek(pager->fileDescriptor, pageNum * PAGE_SIZE, SEEK_SET);

    if (offset == -1) {
        printf("error seeking %d", pageNum);
        exit(EXIT_FAILURE);
    }

    ssize_t bytesWritten = write(pager->fileDescriptor, pager->pages[pageNum], size);

    if (bytesWritten == -1) {
        printf("could not write page to file");
        exit(EXIT_FAILURE);
    }
}
