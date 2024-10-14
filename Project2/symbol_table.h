// symbol_table.h

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

// Symbol structure
typedef struct Symbol {
    char* name;
    char* type;
    int intValue;
    struct Symbol* next;
} Symbol;

// Symbol Table structure
typedef struct SymbolTable {
    int size;
    struct Symbol** table;
} SymbolTable;

// Function prototypes
SymbolTable* createSymbolTable(int size);
void addSymbol(SymbolTable* table, char* name, char* type, int value);
Symbol* lookupSymbol(SymbolTable* table, char* name);
void freeSymbolTable(SymbolTable* table);
void printSymbolTable(const SymbolTable* table);

#endif // SYMBOL_TABLE_H
