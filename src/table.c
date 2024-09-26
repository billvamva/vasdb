#include "table.h"
#include "common.h"
#include "row.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Table* InitDatabase(const char* fileName)
{
    Pager* pager = OpenPager(fileName);
    Table* table = (Table*)malloc(sizeof(Table));
    uint32_t numRows = pager->fileLength / sizeof(Row);
    table->numRows = numRows;
    table->pager = pager;

    return table;
}

void CloseDatabase(Table* table)
{
    uint32_t numFullPages = table->numRows / ROWS_PER_PAGE;
    for (uint32_t i = 0; i < numFullPages; i++) {
        if (table->pager->pages[i] == NULL) {
            continue;
        }
        FlushPager(table->pager, i, PAGE_SIZE);
        free(table->pager->pages[i]);
        table->pager->pages[i] = NULL;
    }
    // TODO: Remove when B tree is implemented
    uint32_t numAdditionalRows = table->numRows % ROWS_PER_PAGE;
    if (numAdditionalRows > 0) {
        uint32_t pageNum = numFullPages;
        if (table->pager->pages[pageNum] != NULL) {
            FlushPager(table->pager, pageNum, numAdditionalRows * sizeof(Row));
            free(table->pager->pages[pageNum]);
            table->pager->pages[pageNum] = NULL;
        }
    }

    int cd = close(table->pager->fileDescriptor);

    if (cd == -1) {
        printf("error closing db file \n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void* page = table->pager->pages[i];

        if (page) {
            free(page);
            table->pager->pages[i] = NULL;
        }
    }

    free(table->pager);
    free(table);
}

void* GetCursorPosition(Cursor* cursor)
{
    uint32_t pageNum = cursor->rowNum / ROWS_PER_PAGE;
    void* page = GetPage(cursor->table->pager, pageNum);
    uint32_t rowOffset = cursor->rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * sizeof(Row);

    return page + byteOffset;
}

Cursor* CreateStartCursor(Table* table)
{
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));

    cursor->rowNum = 0;
    cursor->table = table;
    cursor->endOfTable = (table->numRows == 0);

    return cursor;
}

Cursor* CreateEndCursor(Table* table)
{
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));

    cursor->rowNum = table->numRows;
    cursor->table = table;
    cursor->endOfTable = true;

    return cursor;
}

void AdvanceCursor(Cursor* cursor)
{
    cursor->rowNum++;

    if (cursor->rowNum >= cursor->table->numRows)
        cursor->endOfTable = true;
}
