#include "inputBuffer.h"
#include "minunit.h"
#include <assert.h>
#include <stdio.h>

int tests_run = 0;

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

static char* all_tests()
{
    mu_run_test(test_new_input_buffer);
    return 0;
}

int main()
{
    char* result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
