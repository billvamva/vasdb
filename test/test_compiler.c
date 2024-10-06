#include "btree.h"
#include "common.h"
#include "execution.h"
#include "inputBuffer.h"
#include "minunit.h"
#include "row.h"
#include "statement.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static char* test_prepare_insert()
{
    InputBuffer input = { .buffer = "insert 1 user1 user1@example.com" };
    Statement statement;
    PrepareResult result = PrepareStatement(&input, &statement);
    mu_assert("prepare insert failed", result == PREPARE_SUCCESS);
    mu_assert("wrong statement type", statement.StatementType == STATEMENT_INSERT);
    mu_assert("wrong id", statement.RowOperation.Insert->id == 1);
    mu_assert("wrong username", strcmp(statement.RowOperation.Insert->username, "user1") == 0);
    mu_assert("wrong email", strcmp(statement.RowOperation.Insert->email, "user1@example.com") == 0);
    return 0;
}

static char* test_prepare_select()
{
    InputBuffer input = { .buffer = "select" };
    Statement statement;
    PrepareResult result = PrepareStatement(&input, &statement);
    mu_assert("prepare select failed", result == PREPARE_SUCCESS);
    mu_assert("wrong statement type", statement.StatementType == STATEMENT_SELECT);
    return 0;
}

static char* test_prepare_syntax_error()
{
    InputBuffer input = { .buffer = "insert 1 user1" }; // Missing email
    Statement statement;
    PrepareResult result = PrepareStatement(&input, &statement);
    mu_assert("didn't catch syntax error", result == PREPARE_SYNTAX_ERROR);
    return 0;
}

static char* test_prepare_unrecognized_statement()
{
    InputBuffer input = { .buffer = "update 1 newname" };
    Statement statement;
    PrepareResult result = PrepareStatement(&input, &statement);
    mu_assert("didn't catch unrecognized statement", result == PREPARE_FAILURE);
    return 0;
}

static char* test_execute_insert()
{
    char fileName[] = "tempXXXXXX";
    int fd = mkstemp(fileName);

    Table* table = InitDatabase(fileName);
    Statement statement = {
        .StatementType = STATEMENT_INSERT,
        .RowOperation.Insert = &(Row) { 1, "user1", "user1@example.com" }
    };
    ExecuteResult result = ExecuteInsert(&statement, table);
    mu_assert("execute insert failed", result == EXECUTE_SUCCESS);
    mu_assert("root page not created", table->rootPageNum == 0);
    mu_assert("page number not incremented", table->pager->numPages == 1);

    Statement duplicate_insert_statement = {
        .StatementType = STATEMENT_INSERT,
        .RowOperation.Insert = &(Row) { 1, "user2", "user2@example.com" }
    };
    ExecuteResult duplicateResult = ExecuteInsert(&duplicate_insert_statement, table);
    mu_assert("duplicate key not detected", duplicateResult == EXECUTE_DUPLICATE_KEY);

    CloseDatabase(table);
    close(fd);
    unlink(fileName);
    return 0;
}

static char* test_execute_select()
{
    char fileName[] = "tempXXXXXX";
    int fd = mkstemp(fileName);

    Table* table = InitDatabase(fileName);
    Statement insert_statement = {
        .StatementType = STATEMENT_INSERT,
        .RowOperation.Insert = &(Row) { 1, "user1", "user1@example.com" }
    };
    ExecuteInsert(&insert_statement, table);

    Statement select_statement = { .StatementType = STATEMENT_SELECT };
    ExecuteResult result = ExecuteStatement(&select_statement, table);
    mu_assert("execute select failed", result == EXECUTE_SUCCESS);

    CloseDatabase(table);
    close(fd);
    unlink(fileName);
    return 0;
}

static char* test_leaf_node_split()
{
    char fileName[] = "tempXXXXXX";
    int fd = mkstemp(fileName);
    Table* table = InitDatabase(fileName);
    NodeLayout layout = InitNodeLayout();

    // Insert enough rows to cause at least one split
    uint32_t num_insertions = layout.LEAF_NODE_MAX_CELLS + 1;

    for (uint32_t i = 0; i < num_insertions; i++) {
        char username[32], email[255];
        snprintf(username, sizeof(username), "user%d", i);
        snprintf(email, sizeof(email), "user%d@example.com", i);

        Row row = { i, "", "" };
        strncpy(row.username, username, sizeof(row.username) - 1);
        strncpy(row.email, email, sizeof(row.email) - 1);

        Statement statement = {
            .StatementType = STATEMENT_INSERT,
            .RowOperation.Insert = &row
        };

        ExecuteResult result = ExecuteInsert(&statement, table);
        mu_assert("Insert should succeed", result == EXECUTE_SUCCESS);
    }

    // Verify that we have more than one page
    mu_assert("Should have more than one page after split", table->pager->numPages > 1);

    // Verify the content of the first two pages
    void* page1 = GetPage(table->pager, 0);
    void* page2 = GetPage(table->pager, 1);

    uint32_t cells_page1 = *GetLeafNodeNumCells(page1, &layout);
    uint32_t cells_page2 = *GetLeafNodeNumCells(page2, &layout);

    mu_assert("First page should be full", cells_page1 == layout.LEAF_NODE_MAX_CELLS / 2);
    mu_assert("Second page should have the rest of the cells",
        cells_page1 + cells_page2 == num_insertions);

    CloseDatabase(table);
    close(fd);
    unlink(fileName);
    return 0;
}

static char* test_prepare_insert_long_strings()
{
    char long_username[COLUMN_USERNAME_SIZE + 10];
    char long_email[COLUMN_EMAIL_SIZE + 10];
    memset(long_username, 'a', sizeof(long_username) - 1);
    memset(long_email, 'b', sizeof(long_email) - 1);
    long_username[sizeof(long_username) - 1] = '\0';
    long_email[sizeof(long_email) - 1] = '\0';

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "insert 1 %s %s", long_username, long_email);

    InputBuffer input = { .buffer = buffer };
    Statement statement;
    PrepareResult result = PrepareStatement(&input, &statement);
    mu_assert("didn't catch long strings", result == PREPARE_STRING_TOO_LONG);

    return 0;
}

char* run_compiler_tests()
{
    mu_run_test(test_prepare_insert);
    mu_run_test(test_prepare_select);
    mu_run_test(test_prepare_syntax_error);
    mu_run_test(test_prepare_unrecognized_statement);
    mu_run_test(test_execute_insert);
    mu_run_test(test_execute_select);
    mu_run_test(test_leaf_node_split);
    mu_run_test(test_prepare_insert_long_strings);
    return 0;
}
