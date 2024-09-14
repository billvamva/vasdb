#include "sqlcompiler.h"
#include "inputBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void ExecuteStatement(Statement* statement)
{
    switch (statement->StatementType) {
    case STATEMENT_INSERT:
        printf("this is an insert statement with args %d, %s, %s\n",
            statement->RowOperation.Insert->id,
            statement->RowOperation.Insert->username,
            statement->RowOperation.Insert->email);
    case STATEMENT_SELECT:
        printf("this is a select statement\n");
    }
}
