#ifndef STATEMENT_H
#define STATEMENT_H

#include "row.h"

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef struct {
    StatementType StatementType;
    union {
        Row* Insert;
    } RowOperation;
} Statement;

void InsertDataToStatement(Statement*, int, const char*, const char*);
int IsValidId(const char*);
int AreStringsValid(const char*, const char*);
void FreeStatement(Statement* statement);

#endif /* STATEMENT_H */
