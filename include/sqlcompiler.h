#include "inputBuffer.h"

#ifndef SQL_COMPILER_H
#define SQL_COMPILER_H
#define COLUMN_USERNAME_SIZE 32 // varchar(32)
#define COLUMN_EMAIL_SIZE 255 // varchar(255)

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_FAILURE,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef struct {
    int id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
    StatementType StatementType;
    union {
        Row* Insert;
    } RowOperation;
} Statement;

PrepareResult PrepareStatement(InputBuffer*, Statement*);
void ExecuteStatement(Statement*);

#endif /* ifndef SQL_COMPILER_H */
