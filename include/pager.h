#ifndef PAGER_H
#define PAGER_H

#include <stdint.h>

#define TABLE_MAX_PAGES 100

typedef struct {
    int fileDescriptor;
    uint32_t fileLength;
    void* pages[TABLE_MAX_PAGES];
} Pager;

Pager* OpenPager(const char*);
void* GetPage(Pager*, int);
void FlushPager(Pager*, uint32_t, uint32_t);

#endif /* ifndef PAGER_H */
