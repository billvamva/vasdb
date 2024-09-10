#include <stddef.h>
#include <sys/types.h>

#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

typedef struct {
    char* buffer;
    size_t bufferSize;
    ssize_t inputSize;
} InputBuffer;

InputBuffer* NewInputBuffer();
void PrintPrompt();
void ReadInput(InputBuffer*);
void CloseInputBuffer(InputBuffer*);

#endif /* ifndef INPUT_BUFFER_H */
