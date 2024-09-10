#include "inputBuffer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    InputBuffer* inputBuffer = NewInputBuffer();

    while (true) {
        PrintPrompt();
        ReadInput(inputBuffer);

        if (strcmp(inputBuffer->buffer, ".exit") == 0) {
            CloseInputBuffer(inputBuffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("unrecognised command %s\n", inputBuffer->buffer);
        }
    }
}
