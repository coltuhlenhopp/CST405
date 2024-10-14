#include "sem.h"


#include <stdio.h>

TAC* tHead = NULL;
TAC* tTail = NULL;
int tempVarCount = 0;

// function to create a temporary variable
char* genTempVar() {
    char* variable = (char*)malloc(10 * sizeof(char));
    sprintf(variable, "t%d", tempVarCount++);
    return variable;
}

// Function to generate TAC (Three-Address Code) for an expression
// Parameters: 
// - expr: the ASTNode representing the expression
// - symTab: the symbol table to look up variable declarations
// Returns: a temporary variable or symbol name representing the result of the expression
char* genExprTAC(ASTNode* expr, SymbolTable* symTab) {
    // If the node is a simple expression (like a number)
    if (expr->type == NodeType_SimpleExpr) {
        // Generate a temporary variable to hold the value
        char* tempVar = genTempVar();
        
        // Buffer to hold the string representation of the number
        char valueStr[20];
        
        // Convert the number to a string
        sprintf(valueStr, "%d", expr->simpleExpr.number);
        
        // Generate TAC for moving the number into the temporary variable
        genTAC("MOV", tempVar, valueStr, NULL);  
        
        // Return the temporary variable holding the number
        return tempVar;
    } 
    // If the node is a simple identifier (like a variable)
    else if (expr->type == NodeType_SimpleID) {
        // Look up the variable in the symbol table
        Symbol* sym = lookupSymbol(symTab, expr->simpleID.name);
        
        // If the variable exists, return its name
        if (sym != NULL) {
            return sym->name;  // Return the variable name directly
        } 
        // If the variable does not exist, output an error and exit
        else {
            fprintf(stderr, "Error: Variable %s used before declaration.\n", expr->simpleID.name);
            exit(EXIT_FAILURE);
        }
    } 
    // If the node is an expression (like a binary operation)
    else if (expr->type == NodeType_Expr) {
        // Recursively generate TAC for the left and right sub-expressions
        char* leftVar = genExprTAC(expr->expr.left, symTab);
        char* rightVar = genExprTAC(expr->expr.right, symTab);
        
        // Generate a new temporary variable to hold the result of the operation
        char* tempVar = genTempVar();

        // Handle the addition operator
        if (strcmp(expr->expr.operators, "+") == 0) {
            // Generate TAC for adding the two sub-expressions
            genTAC("ADD", tempVar, leftVar, rightVar);  // Correct order of operands
            return tempVar;
        }
        // TODO: Handle other operators (e.g., -, *, /) as needed
    }

    // If no valid expression type is found, return NULL as a default
    return NULL;
}


// Function to generate a new Three-Address Code (TAC) instruction
// Parameters:
// - operation: the TAC operation (e.g., "ADD", "MOV")
// - result: the result variable for the operation
// - oper1: the first operand (if any) for the operation
// - oper2: the second operand (if any) for the operation
void genTAC(const char* operation, const char* result, const char* oper1, const char* oper2) {
    // Allocate memory for a new TAC structure
    TAC* newTAC = (TAC*)malloc(sizeof(TAC));
    
    // Check for memory allocation failure
    if (newTAC == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for TAC.\n");
        exit(EXIT_FAILURE);  // Exit if allocation fails
    }

    // Duplicate the operation and result strings
    newTAC->operation = strdup(operation);  // strdup allocates memory and copies the string
    newTAC->result = strdup(result);
    
    // Duplicate operand 1 if provided, otherwise set to NULL
    newTAC->oper1 = oper1 ? strdup(oper1) : NULL;
    
    // Duplicate operand 2 if provided, otherwise set to NULL
    newTAC->oper2 = oper2 ? strdup(oper2) : NULL;

    // Set the next TAC instruction to NULL (this will be the end of the list for now)
    newTAC->next = NULL;

    // If the TAC list is empty (tTail is NULL), set tHead to the new TAC
    if (tTail == NULL) {
        tHead = newTAC;  // First TAC node in the list
    } 
    // Otherwise, append the new TAC to the end of the list
    else {
        tTail->next = newTAC;
    }

    // Update tTail to point to the new TAC (the last node in the list)
    tTail = newTAC;
}

// Function to print the TAC
void printTAC() {
    static int tacWrittenToFile = 0;  
    TAC* current = tHead;
    FILE* tacFile = NULL;

    // Only open the file and write if it's the first time this function is called
    if (!tacWrittenToFile) {
        tacFile = fopen("TAC.ir", "w");  // Open the file for writing
        if (tacFile == NULL) {
            fprintf(stderr, "Error: Could not open TAC.ir for writing.\n");
            return;
        }
        tacWrittenToFile = 1;  // Set the flag to prevent further file writes
    }

    while (current != NULL) {
        // Check nulls
        const char* result = current->result ? current->result : "";
        const char* oper1 = current->oper1 ? current->oper1 : "";
        const char* operation = current->operation ? current->operation : "";
        const char* oper2 = current->oper2 ? current->oper2 : "";

       // Print format depending on the operation type
        if (strcmp(operation, "MOV") == 0) {
            // For MOV operations, print "result = MOV operand1"
            printf("%s = %s %s\n", result, operation, oper1);
            if (tacFile) fprintf(tacFile, "%s = %s %s\n", result, operation, oper1);
        } 
        // If there are two operands, print "result = operand1 operation operand2"
        else if (strlen(oper2) > 0) {
            printf("%s = %s %s %s\n", result, oper1, operation, oper2);
            if (tacFile) fprintf(tacFile, "%s = %s %s %s\n", result, oper1, operation, oper2);
        } 
        // For single operand operations (like MOV), print "result = operation operand1"
        else {
            printf("%s = %s %s\n", result, operation, oper1);
            if (tacFile) fprintf(tacFile, "%s = %s %s\n", result, operation, oper1);
        }

        current = current->next;
    }

    if (tacFile) {
        fclose(tacFile);  
    }
}

void printOptimizedTAC() {
    static int optimizedTACWrittenToFile = 0; 
    TAC* current = tHead; // Assuming tacHead points to the optimized TAC after optimization
    FILE* tacFile = NULL;

    // Only open the file and write if it's the first time this function is called
    if (!optimizedTACWrittenToFile) {
        tacFile = fopen("BetterTAC.ir", "w");  // Open the file for writing
        if (tacFile == NULL) {
            fprintf(stderr, "Could not open BetterTAC.ir for writing.\n");
            return;
        }
        optimizedTACWrittenToFile = 1;  // Set the flag to prevent further file writes
    }

    while (current != NULL) {
        // Ensure that each part of the TAC is checked for NULL to prevent crashes.
        const char* result = current->result ? current->result : "";
        const char* oper1 = current->oper1 ? current->oper1 : "";
        const char* operation = current->operation ? current->operation : "";
        const char* oper2 = current->oper2 ? current->oper2 : "";

        // Print depending on the operation type
        if (strcmp(operation, "MOV") == 0) {
            // MOV operation format: "result = MOV oper1"
            printf("%s = %s %s\n", result, operation, oper1);
            if (tacFile) fprintf(tacFile, "%s = %s %s\n", result, operation, oper1);
        } 
        else if (strlen(oper2) > 0) {
            // Binary operations format: "result = oper1 operation oper2"
            printf("%s = %s %s %s\n", result, oper1, operation, oper2);
            if (tacFile) fprintf(tacFile, "%s = %s %s %s\n", result, oper1, operation, oper2);
        } 
        else {
            // Unary operations format: "result = operation oper1"
            printf("%s = %s %s\n", result, operation, oper1);
            if (tacFile) fprintf(tacFile, "%s = %s %s\n", result, operation, oper1);
        }

        current = current->next;
    }

    if (tacFile) {
        fclose(tacFile);  // Close the file after writing
    }
}

char* detExprType(ASTNode* expr, SymbolTable* symTab) {
    if (!expr) return NULL;  // Return NULL if the expression is NULL

    // Determine the type of the expression based on its node type
    switch (expr->type) {
        case NodeType_SimpleExpr:
            // For simple expressions, we assume the type is 'int'
            return "int";

        case NodeType_SimpleID: {
            // Look up the variable in the symbol table by its name
            Symbol* sym = lookupSymbol(symTab, expr->simpleID.name);
            if (sym) {
                return sym->type;  // Return the type of the variable
            } else {
                // Error if the variable is used before declaration
                fprintf(stderr, "Need to declare Variable %s.\n", expr->simpleID.name);
                exit(EXIT_FAILURE);  // Exit with failure status
            }
        }

        case NodeType_Expr:
            // Recursively determine the type of the left-hand side of the expression
            // Assuming binary operations between the same types, like int + int
            return detExprType(expr->expr.left, symTab);

        // Add cases for other node types as needed, e.g., function calls, array accesses, etc.

        default:
            // Return NULL if the node type is unknown
            return NULL;
    }
}


void semCheck(ASTNode* node, SymbolTable* symTab) {
    if (!node) return;

    switch (node->type) {
        case NodeType_Program:
            //printf("Checking the program...\n");
            semCheck(node->program.varDeclList, symTab);
            semCheck(node->program.stmtList, symTab);
            break;

        case NodeType_VarDeclList:
            //printf("Checking variable declaration list...\n");
            semCheck(node->varDeclList.varDecl, symTab);
            semCheck(node->varDeclList.varDeclList, symTab);
            break;

        case NodeType_VarDecl:
            // Add the variable to the symbol table with its type
            //printf("Adding variable '%s' of type '%s' to the symbol table.\n", node->varDecl.varName, node->varDecl.varType);
            break;

        case NodeType_SimpleExpr:
            // Simple expression (e.g., a number) typically doesn't need checking
            //printf("Simple expression with value: %d\n", node->simpleExpr.number);
            break;

        case NodeType_SimpleID: {
            // Ensure the variable is declared
            //printf("Looking up variable '%s'...\n", node->simpleID.name);
            Symbol* sym = lookupSymbol(symTab, node->simpleID.name);
            if (!sym) {
                fprintf(stderr, "Variable '%s' used before declaration.\n", node->simpleID.name);
                exit(EXIT_FAILURE);
            }
            printf("Variable '%s' found (Type: %s).\n", sym->name, sym->type);
            break;
        }

        case NodeType_Expr:
            // Recursively check the left and right sides of the expression
            //printf("Checking expression with operator '%s'...\n", node->expr.operators);
            semCheck(node->expr.left, symTab);
            semCheck(node->expr.right, symTab);
            break;

        case NodeType_StmtList:
            //printf("Checking statement list...\n");
            semCheck(node->stmtList.stmt, symTab);
            semCheck(node->stmtList.stmtList, symTab);
            break;

        case NodeType_WriteStmt: {
            // Ensure the variable being written is declared
            //printf("Checking write statement for variable '%s'...\n", node->writeStmt.id);
            Symbol* sym = lookupSymbol(symTab, node->writeStmt.id);
            if (!sym) {
                fprintf(stderr, "Variable '%s' used before declaration.\n", node->writeStmt.id);
                exit(EXIT_FAILURE);
            }
            printf("Write statement for variable '%s' (Type: %s).\n", sym->name, sym->type);
            break;
        }

        case NodeType_AssignStmt: {
            // Ensure the variable being assigned to is declared
            //printf("Checking assignment to variable '%s'...\n", node->assignStmt.varName);
            Symbol* sym = lookupSymbol(symTab, node->assignStmt.varName);
            if (!sym) {
                fprintf(stderr, "Semantic Error: Variable '%s' used before declaration.\n", node->assignStmt.varName);
                exit(EXIT_FAILURE);
            }
            printf("Variable '%s' found. (Type: %s).\n", sym->name, sym->type);

            // Perform type checking
            char* exprType = detExprType(node->assignStmt.expr, symTab);
            if (strcmp(sym->type, exprType) != 0) {
                fprintf(stderr, "Type mismatch in assignment to '%s'. Expected: %s, Found: %s.\n",
                        node->assignStmt.varName, sym->type, exprType);
                exit(EXIT_FAILURE);
            }
            //printf("Type check passed for assignment to '%s' (Type: %s).\n", node->assignStmt.varName, sym->type);

            // Recursively check the expression
            semCheck(node->assignStmt.expr, symTab);
            break;
        }

        case NodeType_BinOp:
            // Recursively check the left and right operands of the binary operation
            //printf("Checking binary operation '%s'...\n", node->binOp.operators);
            semCheck(node->binOp.left, symTab);
            semCheck(node->binOp.right, symTab);

            // Perform type checking for binary operations (e.g., both operands should be integers)
            char* leftType = detExprType(node->binOp.left, symTab);
            char* rightType = detExprType(node->binOp.right, symTab);
            if (strcmp(leftType, rightType) != 0) {
                fprintf(stderr, "Type mismatch in binary operation '%s'. Left type: %s, Right type: %s.\n",
                        node->binOp.operators, leftType, rightType);
                exit(EXIT_FAILURE);
            }
            //printf("Type check passed for binary operation '%s'.\n", node->binOp.operators);
            break;

        default:
            fprintf(stderr, "Semantic Error: Unknown node type.\n");
            exit(EXIT_FAILURE);
    }
}
