#include "inputBuffer.h"
#include <stdio.h>
#include <stdlib.h>
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
