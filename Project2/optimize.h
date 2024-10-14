#ifndef OPTIMIZE_H
#define OPTIMIZE_H

// Includes
#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// Functions.
TAC* createTAC(const char* operation, const char* result, const char* oper1, const char* oper2);
void optimizeTAC(TAC** head);
void constantFolding(TAC** head, TAC** opHead);
void copyPropagation(TAC** head, TAC** opHead);
void deadCodeElimination(TAC** head, TAC** opHead);
void renumber(TAC** head);
void replaceVariablesWithTemp(TAC** head);
bool isVariable(const char* str);
bool isTemp(const char* str);
bool isConstant(const char* str);
char* createTempVar();
void appendTAC(TAC** head, TAC* newInstruction);



#endif