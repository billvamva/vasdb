#ifndef ROW_H
#define ROW_H

#include "common.h"

typedef struct {
    int id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

void SerialiseRow(Row* source, void* destination, const Field fields[]);
void DeserialiseRow(void* source, Row* destination, const Field fields[]);

#endif /* ROW_H */
