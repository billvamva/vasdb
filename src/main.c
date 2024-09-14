#include "inputBuffer.h"
#include <assert.h>
#include <sqlcompiler.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    InputBuffer* inputBuffer = NewInputBuffer();

    while (true) {
        PrintPrompt();
        ReadInput(inputBuffer);

        if (inputBuffer->buffer[0] == '.') {
            switch (ExecuteMetaCommand(inputBuffer)) {
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
        }

        ExecuteStatement(statement);
        printf("statement executed\n");
        free(statement);
    }
}
