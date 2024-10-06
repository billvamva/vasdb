#ifndef PAGER_H
#define PAGER_H

#include <stdint.h>

#define TABLE_MAX_PAGES 100

typedef struct {
    int fileDescriptor;
    uint32_t fileLength;
    uint32_t numPages;
    void* pages[TABLE_MAX_PAGES];
} Pager;

Pager* OpenPager(const char*);
void* GetPage(Pager*, uint32_t pageNum);
uint32_t GetUnusedPageNum(Pager*);
void FlushPager(Pager*, uint32_t pageNum);

#endif /* ifndef PAGER_H */
