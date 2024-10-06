#include "execution.h"
#include "btree.h"
#include "common.h"
#include "pager.h"
#include "table.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MetaCommandResult ExecuteMetaCommand(InputBuffer* inputBuffer, Table* table)
{
    if (strcmp(inputBuffer->buffer, ".exit") == 0) {
        CloseInputBuffer(inputBuffer);
        CloseDatabase(table);
        exit(EXIT_SUCCESS);
    } else if (strcmp(inputBuffer->buffer, ".btree") == 0) {
        printf("Tree: \n");
        PrintLeafNode(GetPage(table->pager, 0));
        return META_COMMAND_SUCCESS;
    } else if (strcmp(inputBuffer->buffer, ".constants") == 0) {
        printf("Constants: \n");
        PrintConstants();
        return META_COMMAND_SUCCESS;
    } else {
        printf("unrecognised command %s\n", inputBuffer->buffer);
        return META_COMMAND_UNRECOGNISED;
    }
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
    void* page = GetPage(table->pager, table->rootPageNum);
    NodeLayout layout = InitNodeLayout();
    uint32_t numCells = *GetLeafNodeNumCells(page, &layout);
    if (numCells > layout.LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* row = statement->RowOperation.Insert;
    uint32_t keyToInsert = row->id;
    Cursor* cursor = CreateKeyCursor(table, keyToInsert);
    // Cursor* cursor = CreateEndCursor(table);

    if (cursor->cellNum < numCells) {
        uint32_t keyAtIndex = *GetLeafNodeKey(page, &layout, cursor->cellNum);

        if (keyAtIndex == keyToInsert) {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    InsertToLeafNode(cursor, row->id, row, rowOfFields);

    free(cursor);
    return EXECUTE_SUCCESS;
}

void PrintRow(Row row)
{

    printf("| %-3d | %-20s | %-30s |\n",
        row.id,
        row.username,
        row.email);
}

ExecuteResult ExecuteSelect(Table* table)
{

    Cursor* cursor = CreateStartCursor(table);

    printf("+-----------------+----------------------+--------------------------------+\n");
    printf("| %-3s | %-20s | %-30s |\n", "id", "username", "email");
    printf("+-----------------+----------------------+--------------------------------+\n");
    while (!cursor->endOfTable) {
        Row row;
        DeserialiseRow(GetCursorValue(cursor), &row, rowOfFields);
        PrintRow(row);
        AdvanceCursor(cursor);
    }
    printf("+-----------------+----------------------+--------------------------------+\n");

    free(cursor);
    return EXECUTE_SUCCESS;
}

void FreeStatement(Statement* statement)
{
    free(statement->RowOperation.Insert);
    free(statement);
}
