// symbol_table.c

#include "symbol_table.h"

// Create a new symbol table
SymbolTable* createSymbolTable() {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!table) {
        fprintf(stderr, "Memory allocation failed for SymbolTable\n");
        exit(EXIT_FAILURE);
    }
    table->head = NULL;
    return table;
}

// Add a symbol to the table
// Returns 1 on success, 0 if symbol already exists
int addSymbol(SymbolTable* table, const char* name, const char* type) {
    if (lookupSymbol(table, name) != NULL) {
        // Symbol already exists
        return 0;
    }

    // Create a new symbol
    Symbol* newSymbol = (Symbol*)malloc(sizeof(Symbol));
    if (!newSymbol) {
        fprintf(stderr, "Memory allocation failed for Symbol\n");
        exit(EXIT_FAILURE);
    }
    newSymbol->name = strdup(name);
    newSymbol->type = strdup(type);
    //newSymbol->type = strdup(value);
    newSymbol->next = table->head;
    table->head = newSymbol;
    return 1;
}

// Lookup a symbol in the table
Symbol* lookupSymbol(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

// Print the symbol table
void printSymbolTable(SymbolTable* table) {
    Symbol* current = table->head;
    printf("Symbol Table:\n");
    while (current != NULL) {
        printf("  Name: %s, Type: %s, Value: %d\n", current->name, current->type, current->value);
        current = current->next;
    }
}

// Free the symbol table
void freeSymbolTable(SymbolTable* table) {
    Symbol* current = table->head;
    while (current != NULL) {
        Symbol* temp = current;
        current = current->next;
        free(temp->name);
        free(temp->type);
        free(temp);
    }
    free(table);
}
