#ifndef SEM_H
#define SEM_H

#include "AST1.h"
#include "symbol_table.h"

// TAC STRUCT

typedef struct TAC {
    char* operation;
    char* result;
    char* oper1;
    char* oper2;
    struct TAC* next;
} TAC;

extern TAC* tHead;
extern TAC* tTail;

// Functions to be used
void genTAC(const char* operation, const char* result, const char* oper1, const char* oper2);
void printTAC();
void printOptimizedTAC();
char* genTempVar();
char* genExprTAC(ASTNode* expr, SymbolTable* symTab);
void semCheck(ASTNode* node, SymbolTable* symTab);
char* detExprType(ASTNode* expr, SymbolTable* symTab);

#endif