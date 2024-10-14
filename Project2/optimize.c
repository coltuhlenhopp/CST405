#include "optimize.h"
#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Helper function to create a new TAC node
TAC* createTAC(const char* operation, const char* result, const char* oper1, const char* oper2) {
    TAC* newTAC = (TAC*)malloc(sizeof(TAC));
    if (!newTAC) {
        fprintf(stderr, "Mem failure for new node.\n");
        exit(EXIT_FAILURE);
    }
    newTAC->operation = strdup(operation);
    newTAC->result = result ? strdup(result) : NULL;
    newTAC->oper1 = oper1 ? strdup(oper1) : NULL;
    newTAC->oper2 = oper2 ? strdup(oper2) : NULL;
    newTAC->next = NULL;
    return newTAC;
}


// Main optimization function
void optimizeTAC(TAC** head) {
    TAC* opHead = NULL;

    // Fold, copy propagations, and dead code elimination calls.
    constantFolding(head, &opHead);
    TAC* propagatedHead = NULL;
    copyPropagation(&opHead, &propagatedHead);

    TAC* finalOptimizedHead = NULL;
    deadCodeElimination(&propagatedHead, &finalOptimizedHead);

    // Renumber registers to maintain a clean sequence
    renumber(&finalOptimizedHead);
    *head = finalOptimizedHead;
}


// Function to perform constant folding optimization in the TAC
void constantFolding(TAC** originalHead, TAC** optimizedHead) {
    TAC* current = *originalHead;  // Pointer to traverse the original TAC list

    // Array to map temporary registers to constant values (as strings)
    char* tempConstantMap[100] = {NULL};  // Stores constants for temp registers like t0, t1, etc.

    while (current != NULL) {
        // Check for MOV operation where a constant is assigned to a temp register
        if (strcmp(current->operation, "MOV") == 0 && isConstant(current->oper1)) {
            // Store the constant value in the map, assuming temp vars are in format t0, t1, etc.
            int tempIndex = atoi(&current->result[1]);
            tempConstantMap[tempIndex] = strdup(current->oper1);
        }

        // Handle ADD operation for constant folding
        if (strcmp(current->operation, "ADD") == 0) {
            // Get the indices of the temp registers holding the operands (if any)
            int leftTempIndex = (current->oper1 && current->oper1[0] == 't') ? atoi(&current->oper1[1]) : -1;
            int rightTempIndex = (current->oper2 && current->oper2[0] == 't') ? atoi(&current->oper2[1]) : -1;

            // Perform constant folding if both operands are constants
            if (leftTempIndex != -1 && rightTempIndex != -1 && 
                tempConstantMap[leftTempIndex] && tempConstantMap[rightTempIndex]) {

                // Retrieve the constant values from the map
                int leftValue = atoi(tempConstantMap[leftTempIndex]);
                int rightValue = atoi(tempConstantMap[rightTempIndex]);
                int foldedResult = leftValue + rightValue;  // Perform the constant folding

                // Create a new MOV TAC instruction to store the folded result
                char resultStr[20];
                snprintf(resultStr, sizeof(resultStr), "%d", foldedResult);  // Convert result to string
                TAC* newTAC = createTAC("MOV", current->result, resultStr, NULL);  // New MOV for the folded result
                appendTAC(optimizedHead, newTAC);

                // Update the constant map with the new result
                int tempIndex = atoi(&current->result[1]);
                tempConstantMap[tempIndex] = strdup(resultStr);

            } else {
                // If no folding, simply append the current ADD operation to the optimized list
                TAC* newTAC = createTAC(current->operation, current->result, current->oper1, current->oper2);
                appendTAC(optimizedHead, newTAC);
            }
        } else {
            // Append non-ADD operations (or non-foldable instructions) without modification
            TAC* newTAC = createTAC(current->operation, current->result, current->oper1, current->oper2);
            appendTAC(optimizedHead, newTAC);
        }

        current = current->next;  // Move to the next TAC instruction
    }
}



// Function to propagate copies and eliminate unnecessary MOV instructions
void copyPropagation(TAC** originalHead, TAC** optimizedHead) {
    TAC* current = *originalHead;  // Pointer to traverse the original TAC list
    char* tempCopyMap[100] = {NULL};  // Map to track which temp register holds the value of another temp

    while (current != NULL) {
        // Handle MOV operations where one temp register is copied to another
        if (strcmp(current->operation, "MOV") == 0 && isTemp(current->oper1)) {
            // Track that the current temp result holds the value of oper1
            int tempIndex = atoi(&current->result[1]);
            tempCopyMap[tempIndex] = strdup(current->oper1);

            // Uncomment for debugging output to show copy propagation
            // printf("Propagating copy: %s now holds the value of %s\n", current->result, current->oper1);
        } else {
            // Replace operands with their propagated values if they exist in the map
            char* oper1 = current->oper1;
            char* oper2 = current->oper2;

            int leftTempIndex = (oper1 && oper1[0] == 't') ? atoi(&oper1[1]) : -1;
            int rightTempIndex = (oper2 && oper2[0] == 't') ? atoi(&oper2[1]) : -1;

            // Substitute operand 1 if a propagated value exists in the map
            if (leftTempIndex != -1 && tempCopyMap[leftTempIndex]) {
                oper1 = tempCopyMap[leftTempIndex];
            }

            // Substitute operand 2 if a propagated value exists in the map
            if (rightTempIndex != -1 && tempCopyMap[rightTempIndex]) {
                oper2 = tempCopyMap[rightTempIndex];
            }

            // Special case handling for WRITE operations
            if (strcmp(current->operation, "WRITE") == 0) {
                oper1 = current->result;  // Use the result directly for WRITE operations
            }

            // Create a new TAC with updated operands after propagation
            TAC* newTAC = createTAC(current->operation, current->result, oper1, oper2);
            appendTAC(optimizedHead, newTAC);
        }

        current = current->next;  // Move to the next TAC instruction
    }
}


// Function to eliminate dead code (unnecessary instructions)
void deadCodeElimination(TAC** originalHead, TAC** optimizedHead) {
    TAC* current = *originalHead;
    int usedRegisters[100] = {0};  // Array to track registers used in WRITE or other operations
    int hasWriteOperations = 0;    // Flag to check if there are any WRITE operations

    // First pass: Mark registers used in WRITE or any operations that output values
    while (current != NULL) {
        if (strcmp(current->operation, "WRITE") == 0 && current->result) {
            int regIndex = atoi(&current->result[1]);
            usedRegisters[regIndex] = 1;  // Mark this register as used
            hasWriteOperations = 1;       // Indicate that there is at least one WRITE operation
        }
        current = current->next;
    }

    // Second pass: Append only necessary instructions to the optimized TAC list
    current = *originalHead;
    while (current != NULL) {
        if (strcmp(current->operation, "MOV") == 0 && isTemp(current->result)) {
            int regIndex = atoi(&current->result[1]);
            // Append only if the target temp register is used
            if (usedRegisters[regIndex]) {
                TAC* newTAC = createTAC(current->operation, current->result, current->oper1, current->oper2);
                appendTAC(optimizedHead, newTAC);
            }
        } else {
            // For other instructions (non-MOV), simply append them
            TAC* newTAC = createTAC(current->operation, current->result, current->oper1, current->oper2);
            appendTAC(optimizedHead, newTAC);
        }
        current = current->next;
    }

    // Handle case where there are no WRITE statements and we should preserve the last instruction
    if (!hasWriteOperations && *optimizedHead == NULL && *originalHead != NULL) {
        // Traverse to the last instruction in the original TAC
        current = *originalHead;
        while (current->next != NULL) {
            current = current->next;
        }
        // Append the last TAC instruction to the optimized list
        TAC* newTAC = createTAC(current->operation, current->result, current->oper1, current->oper2);
        appendTAC(optimizedHead, newTAC);
    }
}



// renumbers the registers
void renumber(TAC** head) {
    TAC* current = *head;
    TAC* newHead = NULL;
    TAC* newTail = NULL;

    // Map to track old temp registers to new register numbers
    char* registerMap[100] = {NULL};  // Adjust size if needed
    int regCounter = 0;

    // First Pass: Renumber MOV operations, skipping WRITE initially
    while (current != NULL) {
        char newResult[10] = "";
        char newOper1[10] = "";
        char newOper2[10] = "";

        // Renumber result register if it's a temp variable and update the map
        if (strcmp(current->operation, "MOV") == 0 && isTemp(current->result)) {
            int oldIndex = atoi(&current->result[1]);
            if (!registerMap[oldIndex]) {
                sprintf(newResult, "t%d", regCounter);
                registerMap[oldIndex] = strdup(newResult);
                regCounter++;
            } else {
                strcpy(newResult, registerMap[oldIndex]);
            }
        } else {
            // Preserve non-temp results (e.g., variables, constants)
            strcpy(newResult, current->result ? current->result : "");
        }

        // Renumber operand1 if it's a temp variable
        if (current->oper1 && isTemp(current->oper1)) {
            int oldIndex = atoi(&current->oper1[1]);
            strcpy(newOper1, registerMap[oldIndex] ? registerMap[oldIndex] : current->oper1);
        } else {
            strcpy(newOper1, current->oper1 ? current->oper1 : "");
        }

        // Renumber operand2 if it's a temp variable
        if (current->oper2 && isTemp(current->oper2)) {
            int oldIndex = atoi(&current->oper2[1]);
            strcpy(newOper2, registerMap[oldIndex] ? registerMap[oldIndex] : current->oper2);
        } else {
            strcpy(newOper2, current->oper2 ? current->oper2 : "");
        }

        // Create the new TAC and append it to the new list
        TAC* newTAC = createTAC(current->operation, newResult, newOper1, newOper2);
        if (newHead == NULL) {
            newHead = newTAC;
            newTail = newTAC;
        } else {
            newTail->next = newTAC;
            newTail = newTAC;
        }

        current = current->next;
    }

    // Second Pass: Update WRITE instructions with renumbered registers
    current = newHead;
    while (current != NULL) {
        if (strcmp(current->operation, "WRITE") == 0 && isTemp(current->result)) {
            int writeRegIndex = atoi(&current->result[1]);
            if (registerMap[writeRegIndex]) {
                // Apply the direct renumbering for WRITE instructions
                strcpy(current->result, registerMap[writeRegIndex]);
                current->oper1 = NULL;  // WRITE does not require operands in this case
            }
        }
        current = current->next;
    }

    // Update the original head to point to the renumbered TAC list
    *head = newHead;

    // Free dynamically allocated memory for registerMap
    for (int i = 0; i < 100; i++) {
        if (registerMap[i]) {
            free(registerMap[i]);
        }
    }
}



// Function to replace variables with temp variables.
void replaceVariablesWithTemp(TAC** tacList) {
    TAC* currentTAC = *tacList;
    TAC* newTACListHead = NULL;
    TAC* newTACListTail = NULL;

    // Map of variables to temp variables
    char* varToTempMap[100][2];
    int mapSize = 0;

    while (currentTAC != NULL) {
        if (strcmp(currentTAC->operation, "MOV") == 0 && isVariable(currentTAC->result) && isTemp(currentTAC->oper1)) {
            int found = 0;
            for (int i = 0; i < mapSize; i++) {
                if (strcmp(varToTempMap[i][0], currentTAC->result) == 0) {
                    free(varToTempMap[i][1]);
                    varToTempMap[i][1] = strdup(currentTAC->oper1);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                varToTempMap[mapSize][0] = strdup(currentTAC->result);
                varToTempMap[mapSize][1] = strdup(currentTAC->oper1);
                mapSize++;
            }
            currentTAC = currentTAC->next;
            continue;
        }

        char* operand1 = currentTAC->oper1;
        char* operand2 = currentTAC->oper2;

        // Replace variables in operands
        for (int i = 0; i < mapSize; i++) {
            if (operand1 && strcmp(operand1, varToTempMap[i][0]) == 0) operand1 = varToTempMap[i][1];
            if (operand2 && strcmp(operand2, varToTempMap[i][0]) == 0) operand2 = varToTempMap[i][1];
        }

        // Handle WRITE operation
        if (strcmp(currentTAC->operation, "WRITE") == 0) {
            for (int i = 0; i < mapSize; i++) {
                if (currentTAC->result && strcmp(currentTAC->result, varToTempMap[i][0]) == 0) {
                    currentTAC->result = varToTempMap[i][1];
                    break;
                }
            }
        }

        TAC* newTAC = createTAC(currentTAC->operation, currentTAC->result, operand1, operand2);

        // Append to the new list
        if (newTACListHead == NULL) {
            newTACListHead = newTAC;
            newTACListTail = newTAC;
        } else {
            newTACListTail->next = newTAC;
            newTACListTail = newTAC;
        }

        currentTAC = currentTAC->next;
    }

    // Update the original list
    *tacList = newTACListHead;

    // Free memory for varToTempMap
    for (int i = 0; i < mapSize; i++) {
        free(varToTempMap[i][0]);
        free(varToTempMap[i][1]);
    }
}

// Function to check if a string is a valid variable name
bool isVariable(const char* str) {
    if (str == NULL || *str == '\0') return false;
    if (!isalpha((unsigned char)*str) && *str != '_') return false;
    ++str;
    while (*str) {
        if (!isalnum((unsigned char)*str) && *str != '_') return false;
        ++str;
    }
    return true;
}

// Function to check if a string is a temporary variable (e.g., "t0", "t1", etc.)
bool isTemp(const char* str) {
    return str && str[0] == 't' && isdigit((unsigned char)str[1]);
}

// Function to check if a string is a constant integer
bool isConstant(const char* str) {
    if (str == NULL || *str == '\0') return false;
    if (*str == '-') ++str;  // Handle negative constants
    while (*str) {
        if (!isdigit((unsigned char)*str)) return false;
        ++str;
    }
    return true;
}

// Create a temporary variable for TAC results
char* createTempVar() {
    static int tempCounter = 0;
    char* tempVar = (char*)malloc(10);
    snprintf(tempVar, 10, "t%d", tempCounter++);
    return tempVar;
}

// add node to the list.
void appendTAC(TAC** head, TAC* newInstruction) {
    if (!newInstruction) return; // Prevent appending a NULL instruction
    if (!*head) {
        *head = newInstruction;
    } else {
        TAC* current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = newInstruction;
    }
}