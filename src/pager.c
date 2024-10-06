#include "pager.h"
#include "common.h"
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

    uint32_t numPages = fileLength / PAGE_SIZE;

    if (fileLength % PAGE_SIZE != 0) {
        printf("corrupt data, file length is not a multiplier of page size\n");
        exit(EXIT_FAILURE);
    }

    pager->numPages = numPages;

    for (uint32_t i = 0; i < pager->numPages; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

// Gets or inits page with standard size
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

        if (pageNum < numPages) {
            lseek(pager->fileDescriptor, pageNum * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->fileDescriptor, page, PAGE_SIZE);
            if (bytes_read == -1) {
                printf("could not read db file\n");
                exit(EXIT_FAILURE);
            }
        } else if (pageNum >= pager->numPages) {
            pager->numPages = pageNum + 1;
        }

        pager->pages[pageNum] = page;
    }

    return pager->pages[pageNum];
}

uint32_t GetUnusedPageNum(Pager* pager) { return pager->numPages; };

void FlushPager(Pager* pager, uint32_t pageNum)
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

    ssize_t bytesWritten = write(pager->fileDescriptor, pager->pages[pageNum], PAGE_SIZE);

    if (bytesWritten == -1) {
        printf("could not write page to file");
        exit(EXIT_FAILURE);
    }
}
