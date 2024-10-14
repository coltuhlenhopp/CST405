// AST1.h

#ifndef AST1_H
#define AST1_H

#include <stdlib.h>
#include <stdio.h>
#include "symbol_table.h"

// NodeType enum to differentiate between different kinds of AST nodes
typedef enum { 
    NodeType_Program,
    NodeType_VarDeclList, 
    NodeType_VarDecl, 
    NodeType_SimpleExpr,
    NodeType_SimpleID,
    NodeType_Expr, 
    NodeType_StmtList,
    NodeType_WriteStmt,
    NodeType_AssignStmt,
    NodeType_BinOp,
} NodeType;

// Structure for AST nodes
typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            struct ASTNode* varDeclList;
            struct ASTNode* stmtList;
        } program;

        struct {
            struct ASTNode* varDecl;
            struct ASTNode* varDeclList;
        } varDeclList;

        struct {
            char* varType;
            char* varName;
        } varDecl;

        struct {
            int number;
        } simpleExpr;

        struct {
            char* name;
        } simpleID;

        struct {
            char* operators;
            struct ASTNode* left;
            struct ASTNode* right;
        } expr;

        struct {
            struct ASTNode* stmt;
            struct ASTNode* stmtList;
        } stmtList;

        struct {
            char* id;
        } writeStmt;

        struct {
            char* operators;
            char* varName;
            struct ASTNode* expr;
        } assignStmt;

        struct {
            char* operators;
            struct ASTNode* left;
            struct ASTNode* right;
        } binOp;
    };
} ASTNode;

ASTNode* createNode(NodeType type);
const char* typeToString(NodeType type);
void astSpace(int level);
void freeAST(ASTNode* node);
void traverseAST(ASTNode* node, int level);
int evaluateExpr(ASTNode* expr, SymbolTable* symTab);

#endif // AST1_H
