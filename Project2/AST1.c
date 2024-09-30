// AST1.c

#include "AST1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int indentValue = 2;

void printIndent(int level) {
    for (int i = 0; i < level; i++) {
        printf("--");
    }
}

void traverseAST(ASTNode* node, int level) {
    if (!node) {
        //printIndent(level);
        //printf("NULL\n");
        return;
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
            break;

        case NodeType_Expr:
            if (node->expr.operator) {
                printf("Expr: Operator = %s\n", node->expr.operator);
            } else if (node->expr.varName) {
                printf("Expr: Variable = %s\n", node->expr.varName);
            } else {
                printf("Expr: Number = %d\n", node->expr.number);
            }
            traverseAST(node->expr.left, level + indentValue);
            traverseAST(node->expr.right, level + indentValue);
            break;

        case NodeType_StmtList:
            printf("StmtList\n");
            traverseAST(node->stmtList.stmt, level + indentValue);
            traverseAST(node->stmtList.stmtList, level + indentValue);
            break;

        case NodeType_Stmt:
            if (strcmp(node->stmt.operator, "WRITE") == 0) {
                printf("Stmt: WRITE %s\n", node->stmt.varName);
                // Here you can add code to actually perform the write operation
            } else {
                printf("Stmt: %s %s\n", node->stmt.operator, node->stmt.varName);
                traverseAST(node->stmt.expr, level + indentValue);
            }
            break;

        default:
            printf("Unknown Node Type\n");
            break;
    }
}
/*
void printASTNode(ASTNode* node) {
    if (!node) {
        return;  // Skip NULL nodes without printing
    }

    switch (node->type) {
        case NodeType_Program:
            printf("Program\n");
            printASTNode(node->program.varDeclList); // Print var declaration list if present
            printASTNode(node->program.stmtList);    // Print statement list if present
            break;
            
        case NodeType_VarDeclList:
            printf("----VarDeclList\n");
            printASTNode(node->varDeclList.varDecl); // Print variable declaration if present
            //printASTNode(node->varDeclList.varDeclList); // Print remaining declarations
            break;
            
        case NodeType_VarDecl:
            printf("--------VarDecl: Type = %s, Name = %s\n", node->varDecl.varType, node->varDecl.varName);
            printf("@@\n");
            break;
            
        case NodeType_SimpleExpr:
            printf("------------Expr: Number = %d\n", node->simpleExpr.number);
            printf("@@\n");
            break;
            
        case NodeType_Expr:
            printf("------------Expr: Operator = %s\n", node->expr.operator ? node->expr.operator : "(null operator)");
            printf("----------------Left expression:\n");
            printASTNode(node->expr.left);   // Recursively print left expression
            printf("----------------Right expression:\n");
            printASTNode(node->expr.right);  // Recursively print right expression
            break;
            
        case NodeType_StmtList:
            printf("----StmtList\n");
            printASTNode(node->stmtList.stmt); // Print each statement in the list
            printASTNode(node->stmtList.stmtList); // Print remaining statements
            break;
            
        case NodeType_Stmt:
            printf("--------Stmt: %s %s\n", node->stmt.varName, node->stmt.operator);
            printf("------------Expr:\n");
            printASTNode(node->stmt.expr); // Print associated expression
            printf("@@\n");
            break;
    }
}*/

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

// **Constant Folding Function**
void constantFold(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NodeType_Program:
            constantFold(node->program.varDeclList);
            constantFold(node->program.stmtList);
            break;

        case NodeType_VarDeclList:
            constantFold(node->varDeclList.varDecl);
            constantFold(node->varDeclList.varDeclList);
            break;

        case NodeType_StmtList:
            constantFold(node->stmtList.stmt);
            constantFold(node->stmtList.stmtList);
            break;

        case NodeType_Stmt:
            if (node->stmt.expr) {
                constantFold(node->stmt.expr);
            }
            break;

        case NodeType_Expr:
            // First, recursively fold left and right expressions
            if (node->expr.left) {
                constantFold(node->expr.left);
            }
            if (node->expr.right) {
                constantFold(node->expr.right);
            }

            // Check if current node is a '+' operation with two simple expressions
            if (node->expr.operator && strcmp(node->expr.operator, "+") == 0) {
                if (node->expr.left && node->expr.right &&
                    node->expr.left->type == NodeType_SimpleExpr &&
                    node->expr.right->type == NodeType_SimpleExpr) {
                    
                    int leftVal = node->expr.left->simpleExpr.number;
                    int rightVal = node->expr.right->simpleExpr.number;
                    int result = leftVal + rightVal;

                    printf("Constant folding: %d + %d = %d\n", leftVal, rightVal, result);

                    // Change the current node to a SimpleExpr node with the result
                    node->type = NodeType_SimpleExpr;
                    node->simpleExpr.number = result;

                    // Free the left and right nodes
                    freeAST(node->expr.left);
                    freeAST(node->expr.right);

                    // Nullify operator and variable name
                    node->expr.operator = NULL;
                    node->expr.varName = NULL;
                }
            }
            break;

        case NodeType_SimpleExpr:
            // Nothing to fold
            break;

        default:
            break;
    }
}
