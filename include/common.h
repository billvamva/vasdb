#ifndef COMMON_H
#define COMMON_H

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
    EXECUTE_SUCCESS,
    EXECUTE_DUPLICATE_KEY,
    EXECUTE_TABLE_FULL,
} ExecuteResult;

typedef struct {
    uint32_t size;
    uint32_t offset;
} Field;

#endif /* COMMON_H */
