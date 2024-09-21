#include "sqlcompiler.h"
#include "inputBuffer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / sizeof(Row);
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

PrepareResult PrepareStatement(InputBuffer* inputBuffer, Statement* statement)
{
    if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
        statement->StatementType = STATEMENT_INSERT;
        statement->RowOperation.Insert = (Row*)malloc(sizeof(Row));
        // sample insert 1 vas vas@email.com
        int args_assigned
            = sscanf(inputBuffer->buffer, "insert %d %s %s",
                &(statement->RowOperation.Insert->id),
                statement->RowOperation.Insert->username,
                statement->RowOperation.Insert->email);

        if (args_assigned < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strncmp(inputBuffer->buffer, "select", 6) == 0) {
        statement->StatementType = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_FAILURE;
}

ExecuteResult ExecuteStatement(Statement* statement, Table* table)
{
    switch (statement->StatementType) {
    case STATEMENT_INSERT:
        return ExecuteInsert(statement, table);
        break;

    case STATEMENT_SELECT:
        return ExecuteSelect(table);
        break;
    }

    return EXECUTE_SUCCESS;
}
// Define the row_of_fields array
const Field rowOfFields[] = {
    { .size = sizeof(int), .offset = offsetof(Row, id) },
    { .size = COLUMN_USERNAME_SIZE, .offset = offsetof(Row, username) },
    { .size = COLUMN_EMAIL_SIZE, .offset = offsetof(Row, email) },
    { .size = 0, .offset = 0 }
};

ExecuteResult ExecuteInsert(Statement* statement, Table* table)
{
    if (table->numRows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    printf("this is an insert statement with args %d, %s, %s\n",
        statement->RowOperation.Insert->id,
        statement->RowOperation.Insert->username,
        statement->RowOperation.Insert->email);

    SerialiseRow(statement->RowOperation.Insert, GetRowSlot(table, table->numRows), rowOfFields);
    table->numRows++;
    return EXECUTE_SUCCESS;
}

ExecuteResult ExecuteSelect(Table* table)
{
    Row row;

    for (uint32_t i = 0; i < table->numRows; i++) {

        DeserialiseRow(GetRowSlot(table, i), &row, rowOfFields);
        printf("(id=%d, username=%s, email=%s)\n",
            row.id,
            row.username,
            row.email);
    }
    return EXECUTE_SUCCESS;
}

void FreeStatement(Statement* statement)
{
    free(statement->RowOperation.Insert);
    free(statement);
}

void SerialiseRow(Row* source, void* destination, const Field fields[])
{
    char* dest = (char*)destination;
    const char* src = (char*)source;

    for (size_t i = 0; fields[i].size != 0; i++) {
        memcpy(dest, src + fields[i].offset, fields[i].size);
        dest += fields[i].size;
    }
}

void DeserialiseRow(void* source, Row* destination, const Field fields[])
{
    const char* src = (const char*)source;
    char* dest = (char*)destination;

    for (size_t i = 0; fields[i].size != 0; i++) {
        if (fields[i].size == sizeof(uint32_t)) {
            // For integer fields, copy directly
            memcpy(dest + fields[i].offset, src, fields[i].size);
        } else {
            // For string fields, use strncpy to ensure null-termination
            strncpy(dest + fields[i].offset, src, fields[i].size);
            ((char*)(dest + fields[i].offset))[fields[i].size - 1] = '\0'; // Ensure null-termination
        }
        src += fields[i].size;
    }
}

void* GetRowSlot(Table* table, uint32_t rowNum)
{
    uint32_t pageNum = rowNum / ROWS_PER_PAGE;
    void* page = table->pages[pageNum];
    if (page == NULL) {
        page = table->pages[pageNum] = malloc(PAGE_SIZE);
    }
    uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * sizeof(Row);

    return page + byteOffset;
}

Table* NewTable()
{
    Table* table = (Table*)malloc(sizeof(Table));
    table->numRows = 0;
    for (int i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }

    return table;
}

void FreeTable(Table* table)
{
    for (int i = 0; table->pages[i]; i++) {
        free(table->pages[i]);
    }
    free(table);
}
