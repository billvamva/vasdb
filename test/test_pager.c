#include "common.h"
#include "minunit.h"
#include "pager.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char* test_open_new_file()
{
    const char* testFile = "test_new.db";
    remove(testFile); // Ensure the file doesn't exist
    Pager* pager = OpenPager(testFile);
    mu_assert("error, pager is NULL", pager != NULL);
    mu_assert("error, file descriptor is invalid", pager->fileDescriptor != -1);
    mu_assert("error, file length is not 0", pager->fileLength == 0);
    mu_assert("error, number of pages is not 0", pager->numPages == 0);
    close(pager->fileDescriptor);
    free(pager);
    remove(testFile);
    return 0;
}

// Test opening a file with exact page size
static char* test_open_full_page_file()
{
    const char* testFile = "test_full_page.db";
    int fd = open(testFile, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    char pageData[PAGE_SIZE];
    memset(pageData, 'A', PAGE_SIZE);
    write(fd, pageData, PAGE_SIZE);
    close(fd);

    Pager* pager = OpenPager(testFile);
    mu_assert("error, pager is NULL", pager != NULL);
    mu_assert("error, file descriptor is invalid", pager->fileDescriptor != -1);
    mu_assert("error, file length is incorrect", pager->fileLength == PAGE_SIZE);
    mu_assert("error, number of pages is not 1", pager->numPages == 1);
    close(pager->fileDescriptor);
    free(pager);
    remove(testFile);
    return 0;
}

// Test opening a file with multiple full pages
static char* test_open_multiple_pages_file()
{
    const char* testFile = "test_multiple_pages.db";
    int fd = open(testFile, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    char pageData[PAGE_SIZE];
    memset(pageData, 'A', PAGE_SIZE);
    for (int i = 0; i < 3; i++) {
        write(fd, pageData, PAGE_SIZE);
    }
    close(fd);

    Pager* pager = OpenPager(testFile);
    mu_assert("error, pager is NULL", pager != NULL);
    mu_assert("error, file descriptor is invalid", pager->fileDescriptor != -1);
    mu_assert("error, file length is incorrect", pager->fileLength == PAGE_SIZE * 3);
    mu_assert("error, number of pages is not 3", pager->numPages == 3);
    close(pager->fileDescriptor);
    free(pager);
    remove(testFile);
    return 0;
}

// Test error handling for corrupt file (not a multiple of PAGE_SIZE)
static char* test_open_corrupt_file()
{
    const char* testFile = "test_corrupt.db";
    int fd = open(testFile, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    char data[PAGE_SIZE + 1];
    memset(data, 'A', PAGE_SIZE + 1);
    write(fd, data, PAGE_SIZE + 1);
    close(fd);

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        OpenPager(testFile);
        // If we reach here, OpenPager didn't exit as expected
        fprintf(stderr, "OpenPager failed to exit on corrupt file\n");
        exit(EXIT_SUCCESS);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            mu_assert("OpenPager didn't exit as expected on corrupt file",
                exit_status == EXIT_FAILURE);
        } else if (WIFSIGNALED(status)) {
            mu_assert("OpenPager terminated unexpectedly", 0);
        } else {
            mu_assert("OpenPager behaved unexpectedly", 0);
        }
    } else {
        mu_assert("Fork failed", 0);
    }

    remove(testFile);
    return 0;
}

// Test that all pages are initially set to NULL
static char* test_pages_initially_null()
{
    const char* testFile = "test_pages_null.db";
    Pager* pager = OpenPager(testFile);
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        mu_assert("error, page is not NULL", pager->pages[i] == NULL);
    }
    close(pager->fileDescriptor);
    free(pager);
    remove(testFile);
    return 0;
}

// Run all tests
char* run_pager_tests()
{
    mu_run_test(test_open_new_file);
    mu_run_test(test_open_full_page_file);
    mu_run_test(test_open_multiple_pages_file);
    mu_run_test(test_open_corrupt_file);
    mu_run_test(test_pages_initially_null);
    return 0;
}
