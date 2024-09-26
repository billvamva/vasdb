#include "statement.h"
#include <stdlib.h>
#include <string.h>

void InsertDataToStatement(Statement* statement, int id, const char* username, const char* email)
{
    statement->RowOperation.Insert->id = id;
    strncpy(statement->RowOperation.Insert->username, username, COLUMN_USERNAME_SIZE - 1);
    statement->RowOperation.Insert->username[COLUMN_USERNAME_SIZE - 1] = '\0';
    strncpy(statement->RowOperation.Insert->email, email, COLUMN_EMAIL_SIZE - 1);
    statement->RowOperation.Insert->email[COLUMN_EMAIL_SIZE - 1] = '\0';
}

int IsValidId(const char* id_string)
{
    char* endptr;
    long id = strtol(id_string, &endptr, 10);
    return (*endptr == '\0' && id > 0);
}

int AreStringsValid(const char* username, const char* email)
{
    return (strlen(username) <= COLUMN_USERNAME_SIZE && strlen(email) <= COLUMN_EMAIL_SIZE);
}
