// AST1.h

#ifndef AST1_H
#define AST1_H

#include <stdlib.h>
#include <stdio.h>

// NodeType enum to differentiate between different kinds of AST nodes
typedef enum { 
    NodeType_Program,
    NodeType_VarDeclList, 
    NodeType_VarDecl, 
    NodeType_SimpleExpr,
    NodeType_Expr, 
    NodeType_StmtList,
    NodeType_Stmt 
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
            char* operator;
            struct ASTNode* left;
            struct ASTNode* right;
            char* varName; // For variables
            int number;    // For numbers (if applicable)
        } expr;

        struct {
            struct ASTNode* stmt;
            struct ASTNode* stmtList;
        } stmtList;

        struct {
            char* operator;
            char* varName;
            struct ASTNode* expr;
        } stmt;
    };
} ASTNode;

// Function prototypes for AST handling
ASTNode* createNode(NodeType type);
void freeAST(ASTNode* node);
void traverseAST(ASTNode* node, int level);
void printASTNode(ASTNode* node);
void printIndent(int level);

// **Prototype for Constant Folding**
void constantFold(ASTNode* node);

#endif // AST1_H
