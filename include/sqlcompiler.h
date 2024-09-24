#ifndef SQL_COMPILER_H
#define SQL_COMPILER_H

#include "inputBuffer.h"
#include "pager.h"
#include <stdint.h>

#define COLUMN_USERNAME_SIZE 32 // varchar(32)
#define COLUMN_EMAIL_SIZE 255 // varchar(255)
#define TABLE_MAX_PAGES 100
#define SIZE_OF_ATTRIBUTE(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

extern const uint32_t ROWS_PER_PAGE;
extern const uint32_t TABLE_MAX_ROWS;
extern const uint32_t PAGE_SIZE;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_FAILURE,
    PREPARE_SYNTAX_ERROR,
    PREPARE_MEMORY_ERROR,
    PREPARE_INVALID_ID,
    PREPARE_STRING_TOO_LONG,
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
} ExecuteResult;

typedef struct {
    int id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
    uint32_t size;
    uint32_t offset;
} Field;

typedef struct {
    StatementType StatementType;
    union {
        Row* Insert;
    } RowOperation;
} Statement;

typedef struct {
    uint32_t numRows;
    Pager* pager;
} Table;

MetaCommandResult ExecuteMetaCommand(InputBuffer*, Table*);
void InsertDataToStatement(Statement*, int, const char*, const char*);
int IsValidId(const char*);
int AreStringsValid(const char*, const char*);
PrepareResult PrepareInsert(char*, char*, char*, Statement*);
PrepareResult PrepareStatement(InputBuffer* inputBuffer, Statement* statement);
ExecuteResult ExecuteStatement(Statement* statement, Table* table);
ExecuteResult ExecuteInsert(Statement* statement, Table* table);
ExecuteResult ExecuteSelect(Table* table);
void FreeStatement(Statement* statement);
void SerialiseRow(Row* source, void* destination, const Field fields[]);
void DeserialiseRow(void* source, Row* destination, const Field fields[]);
void* GetRowSlot(Table* table, uint32_t rowNum);
Table* InitDatabase(const char* fileName);
void CloseDatabase(Table* table);

#endif /* SQL_COMPILER_H */
