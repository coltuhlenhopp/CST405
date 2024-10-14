// symbol_table.c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "symbol_table.h"


// must have a hash funciton for symbol table
unsigned int hash(SymbolTable* table, char* name){
    unsigned int h = 0;
    for (; *name != '\0'; name++) h = *name + (h << 5) - h;
    return h % table->size;
}

// Create a new symbol table
SymbolTable* createSymbolTable(int size) {
    SymbolTable* tab = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!tab) {return 0;}
    tab->size = size;
    tab->table = (Symbol**)malloc(sizeof(Symbol*) * size);
    // error check
    if (!tab->table){
        free(tab);
        return 0;
    }
    for (int i = 0; i < size; i++){
        tab->table[i] = 0;
    }
    return tab;
}

// Add a symbol to the table
// Returns 1 on success, 0 if symbol already exists
void addSymbol(SymbolTable* table, char* name, char* type, int value) {
   // check to see if the table has been made
   if (table == NULL || table->table == NULL){
    fprintf(stderr, "Sym Table is not made\n");
   return;
   }

   unsigned int hValue = hash(table, name);

   // check for duplicates
   for (Symbol* symbol = table->table[hValue]; symbol != NULL; symbol = symbol->next){
        if(strcmp(name, symbol->name) == 0) {
            //get rid
            free(symbol->type);
            symbol->type = strdup(type);
            symbol->intValue = value;
            return;
        }
   }

   // Now wee add symbolssssss
   Symbol* sym = (Symbol*)malloc(sizeof(Symbol));
   if (!sym) {
        return;
   }
   sym->name = strdup(name);
   
   //check
   if (!sym->name){
    free(sym);
    return;
   }

   sym->type = strdup(type);
   if (!sym->type) {
    free(sym->name);
    free(sym);
    return;
   }
   sym->intValue = value;
   sym->next = table->table[hValue];
   table->table[hValue] = sym;
}

// // must have a hash funciton for symbol table
// unsigned int hash(SymbolTable* table, char* name){
//     unsigned int h = 0;
//     for (; *name != '\0'; name++) h = *name + (h << 5) - h;
//     return h % table->size;
// }

// Lookup a symbol in the table
Symbol* lookupSymbol(SymbolTable* table, char* name) {
    unsigned int hValue = hash(table, name);

    // searhc for the vlaue
    if (table->table[hValue] == NULL){
        printf("No Symbol in table.\n");
        return NULL;
    }

    // now we search
    for (Symbol* sym = table->table[hValue]; sym != NULL; sym = sym->next){
        printf("Symbol name: %s\n", sym->name);
        if (strcmp(name, sym->name) == 0) return sym;
    }
    return NULL;
}

// Print the symbol table
void printSymbolTable(const SymbolTable* table) {
    printf("\n\nSymbol Table:\n\n");
    
    // For loop to iterate through the sypmbole table
    for (int i = 0; i < table->size; i++){
        Symbol* sym = table->table[i];
        while (sym != 0) {
            // in case of undefined values
            if (sym->intValue == INT_MIN){
                char val[9] = "IDK";
                printf("- Name: %s - Type: %s - Value: %s -\n", sym->name, sym->type, val);
            }
            else {  // if all values are defined
                printf("- Name: %s - Type: %s - Value: %d -\n", sym->name, sym->type, sym->intValue);
                sym = sym->next;
            }
        }
    }
    printf("\nEnd of Symbol Table\n");
}

// Free the symbol table
void freeSymbolTable(SymbolTable* table) {
    for (int i = 0; i < table->size; i++){
        Symbol* symbol = table->table[i];
        while (symbol != 0){
            Symbol* sym = symbol->next;
            free(symbol->name);
            free(symbol->type);
            free(symbol);
            symbol = sym;

        }
    }
    free(table->table);
    free(table);
}
