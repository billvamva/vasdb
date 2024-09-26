#include "execution.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    Cursor* cursor = CreateEndCursor(table);

    SerialiseRow(statement->RowOperation.Insert, GetCursorPosition(cursor), rowOfFields);
    table->numRows++;
    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult ExecuteSelect(Table* table)
{
    Row row;

    Cursor* cursor = CreateStartCursor(table);

    while (!cursor->endOfTable) {

        DeserialiseRow(GetCursorPosition(cursor), &row, rowOfFields);
        printf("(id=%d, username=%s, email=%s)\n",
            row.id,
            row.username,
            row.email);
        AdvanceCursor(cursor);
    }

    free(cursor);
    return EXECUTE_SUCCESS;
}

void FreeStatement(Statement* statement)
{
    free(statement->RowOperation.Insert);
    free(statement);
}
