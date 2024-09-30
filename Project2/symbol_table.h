// symbol_table.h

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Symbol structure
typedef struct Symbol {
    char* name;
    char* type;
    int value;
    struct Symbol* next;
} Symbol;

// Symbol Table structure
typedef struct SymbolTable {
    Symbol* head;
} SymbolTable;

// Function prototypes
SymbolTable* createSymbolTable();
int addSymbol(SymbolTable* table, const char* name, const char* type);
Symbol* lookupSymbol(SymbolTable* table, const char* name);
void printSymbolTable(SymbolTable* table);
void freeSymbolTable(SymbolTable* table);

#endif // SYMBOL_TABLE_H
