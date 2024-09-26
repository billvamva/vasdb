#ifndef TABLE_H
#define TABLE_H

#include "pager.h"
#include <stdbool.h>

typedef struct {
    uint32_t numRows;
    Pager* pager;
} Table;

typedef struct {
    Table* table;
    uint32_t rowNum;
    bool endOfTable;
} Cursor;

Table* InitDatabase(const char* fileName);
void CloseDatabase(Table* table);
void* GetCursorPosition(Cursor* cursor);
Cursor* CreateStartCursor(Table* table);
Cursor* CreateEndCursor(Table* table);
void AdvanceCursor(Cursor* cursor);

#endif /* TABLE_H */
