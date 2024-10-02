// AST1.c

#include "AST1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int indentValue = 2;

TACInstruction tacInstructions[100];  // A simple list to store TAC
int tacIndex = 0;  // To track the number of TAC instructions

void generateMIPS() {
    FILE *tacFile = fopen("tac_output.txt", "r"); // Open the TAC file for reading
    FILE *mipsFile = fopen("mips_output.asm", "w"); // Open the MIPS file for writing
    if (!tacFile) {
        fprintf(stderr, "Error opening TAC file for reading.\n");
        return;
    }
    if (!mipsFile) {
        fprintf(stderr, "Error opening MIPS file for writing.\n");
        fclose(tacFile);
        return;
    }

    char line[256];
    // Writing MIPS file header
    fprintf(mipsFile, ".data\n");
    fprintf(mipsFile, "newline: .asciiz \"\\n\"\n");
    fprintf(mipsFile, "h: .word 0\n");
    fprintf(mipsFile, "g: .word 0\n");
    fprintf(mipsFile, "x: .word 0\n");
    fprintf(mipsFile, ".text\n");
    fprintf(mipsFile, ".globl main\n");
    fprintf(mipsFile, "main:\n");

    while (fgets(line, sizeof(line), tacFile)) {
        char result[10], arg1[10], op[10], arg2[10];
        int count = sscanf(line, "%s = %s %s %s", result, arg1, op, arg2);

        if (count == 4) { // If it's a binary operation
    // Load the first operand into $t0
    fprintf(mipsFile, "    lw $t0, %s\n", arg1); // Load first operand from memory
    // Load the second operand into $t1
    fprintf(mipsFile, "    lw $t1, %s\n", arg2); // Load second operand from memory
    
    // Generate MIPS code for arithmetic operations
    if (strcmp(op, "+") == 0) {
        fprintf(mipsFile, "    add $t2, $t0, $t1\n"); // Add and store result in $t2
    } else if (strcmp(op, "-") == 0) {
        fprintf(mipsFile, "    sub $t2, $t0, $t1\n"); // Subtract and store result in $t2
    } else if (strcmp(op, "*") == 0) {
        fprintf(mipsFile, "    mul $t2, $t0, $t1\n"); // Multiply and store result in $t2
    } else if (strcmp(op, "/") == 0) {
        fprintf(mipsFile, "    div $t0, $t1\n"); // Divide
        fprintf(mipsFile, "    mflo $t2\n"); // Move quotient to $t2
    }

    // Store the result in the appropriate variable
    fprintf(mipsFile, "    sw $t2, %s\n", result); // Store the result in the variable
}

        // Handle WRITE statements
        else if (sscanf(line, "WRITE %s", arg1) == 1) { 
            fprintf(mipsFile, "    lw $a0, %s\n", arg1); // Load the value to print
            fprintf(mipsFile, "    li $v0, 1\n"); // Load syscall for print integer
            fprintf(mipsFile, "    syscall\n");
        } 
        // Handle assignments
        else if (count == 2) { // If it's an assignment
            fprintf(mipsFile, "    sw %s, %s\n", arg1, result); // Store the result in the variable
        } else {
            fprintf(stderr, "Error: Unrecognized TAC line format: %s\n", line);
        }
    }

    // Print final exit syscall
    fprintf(mipsFile, "    li $v0, 10\n"); // Exit syscall
    fprintf(mipsFile, "    syscall\n"); // Exit program

    fclose(tacFile); // Close the TAC file
    fclose(mipsFile); // Close the MIPS file
    printf("MIPS code generated in mips_output.asm\n");
}





void emitTAC(char* result, char* arg1, char* op, char* arg2) {
    // Check for redundancy: skip if result is the same as arg1 and op is NULL
    if (op == NULL && strcmp(result, arg1) == 0) {
        return; // Skip emitting this instruction
    }

    // Create a new TACInstruction
    TACInstruction instr;
    instr.result = strdup(result);
    instr.arg1 = strdup(arg1);
    instr.op = op ? strdup(op) : NULL;
    instr.arg2 = arg2 ? strdup(arg2) : NULL;

    // Add the instruction to the tacInstructions array
    tacInstructions[tacIndex++] = instr;

    // Debug print to ensure TAC is being emitted
    //printf("TAC: %s = %s %s %s\n", result, arg1, op ? op : "", arg2 ? arg2 : "");
}


void printTAC() {
    FILE *file = fopen("tac_output.txt", "w"); // Open a file to write TAC instructions
    if (!file) {
        fprintf(stderr, "Error opening file for writing TAC instructions.\n");
        return;
    }

    printf("\nTAC Instructions:\n\n"); // Print header to console
    //fprintf(file, "TAC Instructions:\n\n"); // Write header to file

    for (int i = 0; i < tacIndex; i++) {
        TACInstruction instr = tacInstructions[i];
        
        // Print to console
        if (instr.op) {
            printf("%s = %s %s %s\n", instr.result, instr.arg1, instr.op, instr.arg2);
            fprintf(file, "%s = %s %s %s\n", instr.result, instr.arg1, instr.op, instr.arg2); // Write to file
        } else if (strcmp(instr.result, "WRITE") == 0) {
            printf("WRITE %s\n", instr.arg1);
            fprintf(file, "WRITE %s\n", instr.arg1); // Write to file
        } else {
            printf("%s = %s\n", instr.result, instr.arg1);
            fprintf(file, "%s = %s\n", instr.result, instr.arg1); // Write to file
        }
    }

    fclose(file); // Close the file
    printf("TAC instructions written to tac_output.txt\n"); // Confirmation message
}


int tempVarIndex = 0; // Global index to manage unique temporary variable names

// Function to create unique temporary variable names
char* newTempVar() {
    char* tempVar = malloc(10); // Allocate space for temp variable name (e.g., "t0", "t1", ...)
    snprintf(tempVar, 10, "t%d", tempVarIndex++);
    return tempVar;
}

void traverseAST(ASTNode* node, int level) {
    if (!node) {
        return; // Ensure we return for null nodes
    }

    printIndent(level);

    switch (node->type) {
        case NodeType_Program:
            printf("Program\n");
            traverseAST(node->program.varDeclList, level + indentValue);
            traverseAST(node->program.stmtList, level + indentValue);
            break;

        case NodeType_VarDeclList:
            printf("VarDeclList\n");
            traverseAST(node->varDeclList.varDecl, level + indentValue);
            traverseAST(node->varDeclList.varDeclList, level + indentValue);
            break;

        case NodeType_VarDecl:
            printf("VarDecl: Type = %s, Name = %s\n", node->varDecl.varType, node->varDecl.varName);
            break;

        case NodeType_SimpleExpr:
            printf("SimpleExpr: %d\n", node->simpleExpr.number);
            char* constTemp = newTempVar(); // Create a new temporary variable for the constant
            if (constTemp) {
                emitTAC(constTemp, (char*)&node->simpleExpr.number, NULL, NULL); // Emit TAC for simple expressions
            } else {
                fprintf(stderr, "Error: Unable to create temporary variable for simple expression\n");
            }
            break;

        case NodeType_Expr:
            if (node->expr.operator) {
                printf("Expr: Operator = %s\n", node->expr.operator);
            } else if (node->expr.varName) {
                printf("Expr: Variable = %s\n", node->expr.varName);
            } else {
                printf("Expr: Number = %d\n", node->expr.number);
            }

            // Recursively traverse left and right expressions if they exist
            char* leftTemp = NULL;
            char* rightTemp = NULL;

            if (node->expr.left) {
                leftTemp = newTempVar(); // Create a temporary variable for the left side
                traverseAST(node->expr.left, level + indentValue);
                emitTAC(leftTemp, "t0", NULL, NULL); // Assuming left result is in t0
            } else {
                //fprintf(stderr, "Error: Left expression is null\n");
            }

            if (node->expr.right) {
                rightTemp = newTempVar(); // Create a temporary variable for the right side
                traverseAST(node->expr.right, level + indentValue);
                emitTAC(rightTemp, "t0", NULL, NULL); // Assuming right result is in t0
            } else {
                //fprintf(stderr, "Error: Right expression is null\n");
            }

            // Emit TAC for binary operation
            if (leftTemp && rightTemp) {
                char* resultTemp = newTempVar(); // Create a temporary variable for the result
                if (resultTemp) {
                    emitTAC(resultTemp, leftTemp, node->expr.operator, rightTemp); // Emit TAC for binary operation
                } else {
                    fprintf(stderr, "Error: Unable to create temporary variable for result\n");
                }
            }
            break;

        case NodeType_StmtList:
            printf("StmtList\n");
            traverseAST(node->stmtList.stmt, level + indentValue);
            traverseAST(node->stmtList.stmtList, level + indentValue);
            break;

        case NodeType_Stmt:
            if (strcmp(node->stmt.operator, "WRITE") == 0) {
                printf("Stmt: WRITE %s\n", node->stmt.varName);
                emitTAC("WRITE", node->stmt.varName, NULL, NULL); // Emit TAC for WRITE statement
            } else {
                printf("Stmt: %s %s\n", node->stmt.operator, node->stmt.varName);
                
                if (node->stmt.expr) {
                    traverseAST(node->stmt.expr, level + indentValue);
                    // Emit TAC for assignments
                    if (strcmp(node->stmt.operator, "=") == 0) {
                        // Use leftTemp to store the last expression's result
                        char* assignedTemp = newTempVar(); // Temporary for the assignment
                        if (assignedTemp) {
                            emitTAC(assignedTemp, "t0", NULL, NULL); // Assuming the result is in t0
                            emitTAC(node->stmt.varName, assignedTemp, NULL, NULL); // Assign the result to the variable
                        } else {
                            fprintf(stderr, "Error: Unable to create temporary variable for assignment\n");
                        }
                    }
                } else {
                    fprintf(stderr, "Error: Expression for assignment is null\n");
                }
            }
            break;

        default:
            printf("Unknown Node Type\n");
            break;
    }
}

void generateTAC(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NodeType_VarDecl:
            // For variable declaration, no TAC needed
            break;

        case NodeType_VarDeclList:
            generateTAC(node->varDeclList.varDecl);
            generateTAC(node->varDeclList.varDeclList);
            break;

        case NodeType_Stmt:
            if (strcmp(node->stmt.operator, "=") == 0) {
                // Generate TAC for assignment statement (e.g., x = expr)
                generateTAC(node->stmt.expr);  // Generate TAC for the expression on the right-hand side
                emitTAC(node->stmt.varName, "t0", NULL, NULL);  // Store result of expression in variable
            } else if (strcmp(node->stmt.operator, "WRITE") == 0) {
                // Generate TAC for write operation (e.g., write x)
                emitTAC("WRITE", node->stmt.varName, NULL, NULL);
            }
            break;

        case NodeType_StmtList:
            generateTAC(node->stmtList.stmt);
            generateTAC(node->stmtList.stmtList);
            break;

        case NodeType_Expr:
            if (node->expr.operator) {
                // Generate TAC for binary operations (e.g., +, -, etc.)
                generateTAC(node->expr.left);  // Generate TAC for left operand
                char* leftTemp = "t0";  // Assume result is in t0

                generateTAC(node->expr.right);  // Generate TAC for right operand
                char* rightTemp = "t0";  // Assume result is in t0

                emitTAC("t0", leftTemp, node->expr.operator, rightTemp);  // Generate TAC for binary operation
            } else if (node->expr.varName) {
                // Generate TAC for variable access (e.g., x)
                emitTAC("t0", node->expr.varName, NULL, NULL);  // t0 will hold the variable value
            } else {
                // Generate TAC for constants (e.g., numbers)
                char temp[16];
                snprintf(temp, sizeof(temp), "%d", node->expr.number);
                emitTAC("t0", temp, NULL, NULL);  // t0 will hold the constant value
            }
            break;

        default:
            fprintf(stderr, "Error: Unrecognized node type for TAC generation.\n");
            break;
    }
}


void printIndent(int level) {
    for (int i = 0; i < level; i++) {
        printf("--");
    }
}

void freeAST(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NodeType_Program:
            freeAST(node->program.varDeclList);
            freeAST(node->program.stmtList);
            break;
        case NodeType_VarDeclList:
            freeAST(node->varDeclList.varDecl);
            freeAST(node->varDeclList.varDeclList);
            break;
        case NodeType_VarDecl:
            free(node->varDecl.varType);
            free(node->varDecl.varName);
            break;
        case NodeType_SimpleExpr:
            // No dynamic memory to free
            break;
        case NodeType_Expr:
            free(node->expr.operator);
            free(node->expr.varName);
            freeAST(node->expr.left);
            freeAST(node->expr.right);
            break;
        case NodeType_StmtList:
            freeAST(node->stmtList.stmt);
            freeAST(node->stmtList.stmtList);
            break;
        case NodeType_Stmt:
            free(node->stmt.varName);
            free(node->stmt.operator);
            freeAST(node->stmt.expr);
            break;
    }

    free(node);
}

ASTNode* createNode(NodeType type) {
    ASTNode* newNode = (ASTNode*)malloc(sizeof(ASTNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for ASTNode\n");
        exit(EXIT_FAILURE);
    }
    newNode->type = type;

    switch (type) {
        case NodeType_Program:
            newNode->program.varDeclList = NULL;
            newNode->program.stmtList = NULL;
            break;
        case NodeType_VarDeclList:
            newNode->varDeclList.varDecl = NULL;
            newNode->varDeclList.varDeclList = NULL;
            break;
        case NodeType_VarDecl:
            newNode->varDecl.varType = NULL;
            newNode->varDecl.varName = NULL;
            break;
        case NodeType_SimpleExpr:
            newNode->simpleExpr.number = 0;
            break;
        case NodeType_Expr:
            newNode->expr.operator = NULL;  // Initialize as NULL
            newNode->expr.left = NULL;
            newNode->expr.right = NULL;
            newNode->expr.varName = NULL;
            newNode->expr.number = 0;
            break;
        case NodeType_StmtList:
            newNode->stmtList.stmt = NULL;
            newNode->stmtList.stmtList = NULL;
            break;
        case NodeType_Stmt:
            newNode->stmt.varName = NULL;
            newNode->stmt.operator = NULL;
            newNode->stmt.expr = NULL;
            break;
        default:
            break;
    }
    return newNode;
}

