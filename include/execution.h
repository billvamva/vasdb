#ifndef EXECUTION_H
#define EXECUTION_H

#include "common.h"
#include "inputBuffer.h"
#include "statement.h"
#include "table.h"

MetaCommandResult ExecuteMetaCommand(InputBuffer*, Table*);
PrepareResult PrepareInsert(char*, char*, char*, Statement*);
PrepareResult PrepareStatement(InputBuffer* inputBuffer, Statement* statement);
ExecuteResult ExecuteStatement(Statement* statement, Table* table);
ExecuteResult ExecuteInsert(Statement* statement, Table* table);
ExecuteResult ExecuteSelect(Table* table);
void FreeStatement(Statement* statement);

#endif /* EXECUTION_H */
