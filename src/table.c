#include "table.h"
#include "btree.h"
#include "common.h"
#include "pager.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Table* InitDatabase(const char* fileName)
{
    Pager* pager = OpenPager(fileName);
    Table* table = (Table*)malloc(sizeof(Table));
    table->pager = pager;
    table->rootPageNum = 0;

    if (table->pager->numPages == 0) {
        void* node = GetPage(table->pager, table->rootPageNum);
        NodeLayout layout = InitNodeLayout();
        InitialiseLeafNode(node, &layout);
    }

    return table;
}

void CloseDatabase(Table* table)
{
    for (uint32_t i = 0; i < table->pager->numPages; i++) {
        if (table->pager->pages[i] == NULL) {
            continue;
        }
        FlushPager(table->pager, i);
        free(table->pager->pages[i]);
        table->pager->pages[i] = NULL;
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
    void* page = GetPage(cursor->table->pager, cursor->pageNum);
    return page;
}

/*
Creates cursor at the start of the table
*/
Cursor* CreateStartCursor(Table* table)
{
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));

    cursor->table = table;
    cursor->pageNum = table->rootPageNum;
    cursor->cellNum = 0;

    void* page = GetCursorPosition(cursor);
    NodeLayout layout = InitNodeLayout();
    uint32_t numCells = *(GetLeafNodeNumCells(page, &layout));

    cursor->endOfTable = (numCells == 0);

    return cursor;
}

// Creating Cursor in the position where the key would be inserted
Cursor* CreateKeyCursor(Table* table, uint32_t key)
{
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));
    uint32_t rootPageNum = table->rootPageNum;
    void* rootNode = GetPage(table->pager, rootPageNum);
    cursor->cellNum = FindLeafNodeIndex(rootNode, key);
    cursor->table = table;
    cursor->pageNum = rootPageNum;

    return cursor;
}

Cursor* CreateEndCursor(Table* table)
{
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));

    cursor->table = table;
    cursor->pageNum = table->rootPageNum;
    cursor->endOfTable = true;

    NodeLayout layout = InitNodeLayout();
    void* page = GetPage(table->pager, cursor->pageNum);

    uint32_t cellNum = *GetLeafNodeNumCells(page, &layout);

    cursor->cellNum = cellNum;

    return cursor;
}

void* GetCursorValue(Cursor* cursor)
{
    uint32_t pageNum = cursor->pageNum;

    void* node = GetPage(cursor->table->pager, pageNum);
    NodeLayout layout = InitNodeLayout();

    return GetLeafNodeValue(node, &layout, cursor->cellNum);
}

void AdvanceCursor(Cursor* cursor)
{
    uint32_t pageNum = cursor->pageNum;
    void* page = GetPage(cursor->table->pager, pageNum);
    NodeLayout layout = InitNodeLayout();

    cursor->cellNum += 1;

    if (cursor->cellNum >= *GetLeafNodeNumCells(page, &layout)) {
        cursor->endOfTable = true;
    }
}
