#include "inputBuffer.h"
#include "minunit.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char* test_new_input_buffer()
{
    InputBuffer* buffer = NewInputBuffer();
    mu_assert("error, buffer is NULL", buffer != NULL);
    mu_assert("error, buffer->buffer is not NULL", buffer->buffer == NULL);
    mu_assert("error, bufferSize is not 0", buffer->bufferSize == 0);
    mu_assert("error, inputSize is not 0", buffer->inputSize == 0);
    // Don't forget to free the buffer if NewInputBuffer allocates memory
    CloseInputBuffer(buffer);

    return 0;
}

static char* test_read_input()
{
    InputBuffer* buffer = NewInputBuffer();

    // Simulate user input
    FILE* temp = tmpfile();
    fprintf(temp, "test input\n");
    rewind(temp);

    // Redirect stdin to our temp file
    FILE* old_stdin = stdin;
    stdin = temp;

    ReadInput(buffer);

    mu_assert("error, input not read correctly", strcmp(buffer->buffer, "test input") == 0);
    mu_assert("error, inputSize incorrect", buffer->inputSize == 10);
    mu_assert("error, bufferSize not updated", buffer->bufferSize > 0);

    // Restore stdin
    stdin = old_stdin;
    fclose(temp);

    CloseInputBuffer(buffer);
    return 0;
}

char* run_input_buffer_tests()
{
    mu_run_test(test_new_input_buffer);
    mu_run_test(test_read_input);
    return 0;
}
