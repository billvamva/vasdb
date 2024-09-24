#include "sqlcompiler.h"
#include "inputBuffer.h"
#include "pager.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / sizeof(Row);
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

MetaCommandResult ExecuteMetaCommand(InputBuffer* inputBuffer, Table* table)
{
    if (strcmp(inputBuffer->buffer, ".exit") == 0) {
        CloseInputBuffer(inputBuffer);
        CloseDatabase(table);
        exit(EXIT_SUCCESS);
    } else {
        printf("unrecognised command %s\n", inputBuffer->buffer);
        return META_COMMAND_UNRECOGNISED;
    }
}

void InsertDataToStatement(Statement* statement, int id, const char* username, const char* email)
{
    statement->RowOperation.Insert->id = id;
    strncpy(statement->RowOperation.Insert->username, username, COLUMN_USERNAME_SIZE - 1);
    statement->RowOperation.Insert->username[COLUMN_USERNAME_SIZE - 1] = '\0';
    strncpy(statement->RowOperation.Insert->email, email, COLUMN_EMAIL_SIZE - 1);
    statement->RowOperation.Insert->email[COLUMN_EMAIL_SIZE - 1] = '\0';
}

int IsValidId(const char* id_string)
{
    char* endptr;
    long id = strtol(id_string, &endptr, 10);
    return (*endptr == '\0' && id > 0);
}

int AreStringsValid(const char* username, const char* email)
{
    return (strlen(username) <= COLUMN_USERNAME_SIZE && strlen(email) <= COLUMN_EMAIL_SIZE);
}

PrepareResult PrepareInsertStatement(char* id_string, char* username, char* email, Statement* statement)
{
    if (!IsValidId(id_string)) {
        return PREPARE_INVALID_ID;
    }

    if (!AreStringsValid(username, email)) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->RowOperation.Insert = (Row*)malloc(sizeof(Row));
    if (statement->RowOperation.Insert == NULL) {
        return PREPARE_MEMORY_ERROR;
    }

    InsertDataToStatement(statement, atoi(id_string), username, email);
    return PREPARE_SUCCESS;
}

PrepareResult PrepareStatement(InputBuffer* inputBuffer, Statement* statement)
{
    char* tempBuffer = strdup(inputBuffer->buffer);
    if (tempBuffer == NULL) {
        return PREPARE_MEMORY_ERROR;
    }

    char* keyword = strtok(tempBuffer, " ");
    if (keyword == NULL) {
        free(tempBuffer);
        return PREPARE_SYNTAX_ERROR;
    }

    PrepareResult result;
    if (strcmp(keyword, "insert") == 0) {
        statement->StatementType = STATEMENT_INSERT;
        char* id_string = strtok(NULL, " ");
        char* username = strtok(NULL, " ");
        char* email = strtok(NULL, " ");

        if (id_string == NULL || username == NULL || email == NULL) {
            result = PREPARE_SYNTAX_ERROR;
        } else {
            result = PrepareInsertStatement(id_string, username, email, statement);
        }
    } else if (strcmp(keyword, "select") == 0) {
        statement->StatementType = STATEMENT_SELECT;
        result = PREPARE_SUCCESS;
    } else {
        result = PREPARE_FAILURE;
    }

    free(tempBuffer);
    return result;
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
    void* page = GetPage(table->pager, pageNum);
    uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * sizeof(Row);

    return page + byteOffset;
}

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
