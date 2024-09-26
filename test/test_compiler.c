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
    printf("result output: %d \n", result);
    mu_assert("execute insert failed", result == EXECUTE_SUCCESS);
    mu_assert("row count didn't increase", table->numRows == 1);

    Cursor* cursor = CreateStartCursor(table);

    Row* inserted_row = (Row*)GetCursorPosition(cursor);
    mu_assert("inserted id is wrong\n", inserted_row->id == 1);
    mu_assert("inserted username is wrong", strcmp(inserted_row->username, "user1") == 0);
    mu_assert("inserted email is wrong", strcmp(inserted_row->email, "user1@example.com") == 0);

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

static char* test_table_full()
{
    char fileName[] = "tempXXXXXX";
    int fd = mkstemp(fileName);

    Table* table = InitDatabase(fileName);
    Statement statement = {
        .StatementType = STATEMENT_INSERT,
        .RowOperation.Insert = &(Row) { 1, "user1", "user1@example.com" }
    };

    for (uint32_t i = 0; i < TABLE_MAX_ROWS; i++) {
        ExecuteInsert(&statement, table);
    }

    ExecuteResult result = ExecuteInsert(&statement, table);
    mu_assert("didn't catch table full", result == EXECUTE_TABLE_FULL);

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
    mu_run_test(test_table_full);
    mu_run_test(test_prepare_insert_long_strings);
    return 0;
}
