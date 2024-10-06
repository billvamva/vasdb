#ifndef TABLE_H
#define TABLE_H

#include "pager.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    Pager* pager;
    uint32_t rootPageNum;
} Table;

typedef struct {
    Table* table;
    uint32_t pageNum;
    uint32_t cellNum;
    bool endOfTable;
} Cursor;

Table* InitDatabase(const char* fileName);
void CloseDatabase(Table* table);
void* GetCursorPosition(Cursor* cursor);
Cursor* CreateStartCursor(Table* table);
Cursor* CreateEndCursor(Table* table);
Cursor* CreateKeyCursor(Table* table, uint32_t key);
void AdvanceCursor(Cursor* cursor);
void* GetCursorValue(Cursor* cursor);

#endif /* TABLE_H */
