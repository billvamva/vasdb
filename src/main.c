#include "common.h"
#include "execution.h"
#include "inputBuffer.h"
#include "statement.h"
#include "table.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("must supply db file name \n");
        exit(EXIT_FAILURE);
    }
    Table* table = InitDatabase(argv[1]);
    InputBuffer* inputBuffer = NewInputBuffer();

    while (true) {
        PrintPrompt();
        ReadInput(inputBuffer);

        if (inputBuffer->buffer[0] == '.') {
            switch (ExecuteMetaCommand(inputBuffer, table)) {
            case META_COMMAND_SUCCESS:
                continue;
            case META_COMMAND_UNRECOGNISED:
                printf("command not recognised\n");
                continue;
            }
        }

        Statement* statement = (Statement*)malloc(sizeof(Statement));
        assert(statement);
        switch (PrepareStatement(inputBuffer, statement)) {
        case PREPARE_SUCCESS:
            break;
        case PREPARE_FAILURE:
            printf("unrecognised statement\n");
            continue;
        case PREPARE_SYNTAX_ERROR:
            printf("syntax error in statement\n");
            continue;
        case PREPARE_STRING_TOO_LONG:
            printf("string exceeds max length\n");
            continue;
        case PREPARE_INVALID_ID:
            printf("id is invalid\n");
            continue;
        case PREPARE_MEMORY_ERROR:
            printf("memory allocation error\n");
            continue;
        }

        switch (ExecuteStatement(statement, table)) {
        case EXECUTE_SUCCESS:
            printf("Executed. \n");
            break;
        case EXECUTE_TABLE_FULL:
            printf("Error, Table Full");
            break;
        }
        free(statement);
    }
}
