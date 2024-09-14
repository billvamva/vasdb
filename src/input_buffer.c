#include "inputBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

InputBuffer* NewInputBuffer()
{

    InputBuffer* inputBuffer = malloc(sizeof(InputBuffer));
    inputBuffer->buffer = NULL;
    inputBuffer->bufferSize = 0;
    inputBuffer->inputSize = 0;

    return inputBuffer;
}

void PrintPrompt()
{
    printf("vasdb > ");
}

void ReadInput(InputBuffer* inputBuffer)
{
    ssize_t bytesRead = getline(&inputBuffer->buffer, &inputBuffer->bufferSize, stdin);

    if (bytesRead <= 0) {
        printf("error reading input\n");
        exit(EXIT_FAILURE);
    }

    inputBuffer->buffer[bytesRead - 1] = 0;
    inputBuffer->inputSize = bytesRead - 1;
}

void CloseInputBuffer(InputBuffer* inputBuffer)
{
    free(inputBuffer->buffer);
    free(inputBuffer);
}

MetaCommandResult ExecuteMetaCommand(InputBuffer* inputBuffer)
{
    if (strcmp(inputBuffer->buffer, ".exit") == 0) {
        CloseInputBuffer(inputBuffer);
        exit(EXIT_SUCCESS);
    } else {
        printf("unrecognised command %s\n", inputBuffer->buffer);
        return META_COMMAND_UNRECOGNISED;
    }
}
