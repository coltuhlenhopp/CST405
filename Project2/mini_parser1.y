%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST1.h"
#include "symbol_table.h"

int evaluateExpression(ASTNode* expr);
int getSymbolValue(SymbolTable* table, const char* name);
void emitTAC(char* result, char* arg1, char* op, char* arg2);
void printTAC();


SymbolTable* symbolTable;

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

ASTNode* root = NULL; 

%}

%union {
    int number;
    char* string;
    char* operator;
    struct ASTNode* ast;
}

%token <string> TYPE
%token <string> ID
%token SEMICOLON
%token EQ
%token PLUS
%token <number> NUMBER
%token <string> WRITE

%type <ast> Program VarDecl VarDeclList Stmt StmtList Expr
%start Program

%left PLUS

%%

// Program production
Program: VarDeclList StmtList {
    printf("The PARSER has started\n"); 
    $$ = createNode(NodeType_Program);
    $$->program.varDeclList = $1;
    $$->program.stmtList = $2;
    root = $$;
}
;

// Variable declaration list
VarDeclList: VarDecl VarDeclList {  
        printf("PARSER: Recognized variable declaration list\n"); 
        $$ = createNode(NodeType_VarDeclList);
        $$->varDeclList.varDecl = $1;
        $$->varDeclList.varDeclList = $2; // Recursively build the list
    }
| VarDecl {
    printf("PARSER: Recognized final variable declaration\n");
    $$ = createNode(NodeType_VarDeclList);
    $$->varDeclList.varDecl = $1;
    $$->varDeclList.varDeclList = NULL; // End of the declaration list
}
;

// Variable declaration
VarDecl: TYPE ID SEMICOLON { 
    printf("PARSER: Recognized variable declaration: %s\n", $2);
    if (!addSymbol(symbolTable, $2, $1)) {
        fprintf(stderr, "Semantic Error: Variable '%s' redeclared.\n", $2);
        exit(EXIT_FAILURE);
    }
    $$ = createNode(NodeType_VarDecl);
    $$->varDecl.varType = strdup($1);
    $$->varDecl.varName = strdup($2);
}
| TYPE ID error {
    yyerror("Missing semicolon after variable declaration");
    yyerrok; // Recover from error
    $$ = createNode(NodeType_VarDecl);
    $$->varDecl.varType = strdup($1);
    $$->varDecl.varName = strdup($2);
}
;

// Statement list
StmtList: Stmt StmtList { 
        printf("PARSER: Recognized statement list\n");
        $$ = createNode(NodeType_StmtList);
        $$->stmtList.stmt = $1;
        $$->stmtList.stmtList = $2; // Recursively build the list
    }
| Stmt {
    printf("PARSER: Recognized final statement\n");
    $$ = createNode(NodeType_StmtList);
    $$->stmtList.stmt = $1;
    $$->stmtList.stmtList = NULL; // End of the statement list
}
;

// Statement
// Statement
Stmt: ID EQ Expr SEMICOLON {
    printf("PARSER: Recognized assignment statement\n");
    $$ = createNode(NodeType_Stmt);
    $$->stmt.varName = strdup($1);
    $$->stmt.operator = strdup("="); 
    $$->stmt.expr = $3; 

    // Evaluate the expression to get its value
    int value = evaluateExpression($3); // Ensure this function is implemented correctly

    // Look up the symbol in the symbol table
    Symbol* symbol = lookupSymbol(symbolTable, $1);
    //generateTAC("MOV", symbol->name, exprResult, NULL);
    if (symbol != NULL) {
        symbol->value = value; // Assign the evaluated value to the symbol
    } else {
        fprintf(stderr, "Error: Variable '%s' not declared.\n", $1);
        exit(EXIT_FAILURE);
    }
}

| WRITE ID SEMICOLON {
    printf("PARSER: Recognized write statement\n"); 
    $$ = createNode(NodeType_Stmt);
    $$->stmt.varName = strdup($2);
    $$->stmt.operator = strdup("WRITE");
    $$->stmt.expr = NULL;
    //generateTAC("WRITE", $$->stmt.operator, NULL, NULL);
}
;

// Expression production
Expr: Expr PLUS Expr { 
    printf("PARSER: Recognized addition operation\n");
    $$ = createNode(NodeType_Expr);
    $$->expr.operator = strdup("+");
    $$->expr.left = $1;
    $$->expr.right = $3;
    $$->expr.varName = NULL;
}
| ID { 
    printf("PARSER: Recognized variable: %s\n", $1);
    $$ = createNode(NodeType_Expr);
    $$->expr.operator = NULL;
    $$->expr.left = NULL;
    $$->expr.right = NULL;
    $$->expr.varName = strdup($1);
    $$->expr.number = 0;
}
| NUMBER { 
    printf("PARSER: Recognized number: %d\n", $1);
    $$ = createNode(NodeType_Expr);
    $$->expr.operator = NULL;
    $$->expr.left = NULL;
    $$->expr.right = NULL;
    $$->expr.varName = NULL;
    $$->expr.number = $1;
}
| '(' Expr ')' {
    printf("PARSER: Recognized expression in parentheses\n");
    $$ = $2; // Pass the expression inside parentheses up the tree
}
;


%%

int getSymbolValue(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current->value; // Return the value of the symbol
        }
        current = current->next;
    }
    // Handle the case where the symbol is not found
    fprintf(stderr, "Error: Variable '%s' not found in symbol table.\n", name);
    exit(EXIT_FAILURE);
}


int evaluateExpression(ASTNode* expr) {
    if (!expr) return 0; // Handle NULL case

    switch (expr->type) {
        case NodeType_Expr:
            if (expr->expr.varName) {
                // Lookup the variable in the symbol table and return its value
                return getSymbolValue(symbolTable, expr->expr.varName); // Implement this function
            } else if (expr->expr.operator) {
                if (strcmp(expr->expr.operator, "+") == 0) {
                    return evaluateExpression(expr->expr.left) + evaluateExpression(expr->expr.right);
                }
                // Implement other operators as needed (e.g., subtraction, multiplication)
            } else {
                return expr->expr.number; // Return the number directly
            }
            break;
        default:
            printf("Unknown Expression Type\n");
            break;
    }
    return 0; // Default return value if nothing matched
}


void semanticAnalyze(ASTNode* node, SymbolTable* symbolTable) {
    if (!node) return;

    //printf("Analyzing node type: %d\n", node->type);


    switch (node->type) {
        case NodeType_VarDecl:
            // Variables are already added during parsing
            break;

        case NodeType_VarDeclList:
            semanticAnalyze(node->varDeclList.varDecl, symbolTable);
            semanticAnalyze(node->varDeclList.varDeclList, symbolTable);
            break;

        case NodeType_Stmt:
    if (strcmp(node->stmt.operator, "=") == 0) {
        // Check if the variable being assigned to is declared
        Symbol* symbol = lookupSymbol(symbolTable, node->stmt.varName);
        if (symbol == NULL) {
            fprintf(stderr, "Semantic Error: Variable '%s' used before declaration.\n", node->stmt.varName);
            exit(EXIT_FAILURE);
        }
        semanticAnalyze(node->stmt.expr, symbolTable); // Analyze the expression
        printf("Analyzing node type: %d\n", node->type);

        emitTAC(node->stmt.varName, "t0", NULL, NULL);    //================================================

    } else if (strcmp(node->stmt.operator, "WRITE") == 0) {
        // Check if the variable to write is declared
        if (lookupSymbol(symbolTable, node->stmt.varName) == NULL) {
            fprintf(stderr, "Semantic Error: Variable '%s' used before declaration.\n", node->stmt.varName);
            exit(EXIT_FAILURE);
        }
        printf("Analyzing node type: %d\n", node->type);

        emitTAC("WRITE", node->stmt.varName, NULL, NULL); //==============================================

    }
    break;



        case NodeType_StmtList:
            semanticAnalyze(node->stmtList.stmt, symbolTable);
            semanticAnalyze(node->stmtList.stmtList, symbolTable);

            break;

        case NodeType_Expr:
    if (node->expr.operator) {
        printf("Analyzing node type: %d\n", node->type);

        semanticAnalyze(node->expr.left, symbolTable);

        char* leftTemp = "t0"; //===================================================

        semanticAnalyze(node->expr.right, symbolTable);

        char* rightTemp = "t0";

        emitTAC("t0", leftTemp, node->expr.operator, rightTemp); //====================================
        
    } else {
        // Check if it's a variable and if it's declared
        if (node->expr.varName) {
            if (lookupSymbol(symbolTable, node->expr.varName) == NULL) {
                fprintf(stderr, "Semantic Error: Variable '%s' used before declaration.\n", node->expr.varName);
                exit(EXIT_FAILURE);
            }
               // Load variable into a temporary register
                emitTAC("t0", node->expr.varName, NULL, NULL); // t0 will hold the variable value
            } else {
                // Load constant
                char temp[16];
                snprintf(temp, sizeof(temp), "%d", node->expr.number);
                printf("HELLO MOTO");
                emitTAC("t0", temp, NULL, NULL); // t0 will hold the constant
            }
        }
    
    break;


        default:
            break;
    }
}

//======================================================================================================================================

int main(int argc, char** argv) {
    printf("Starting the compiler...\n");
    symbolTable = createSymbolTable();
    printf("Symbol table created.\n");

    // Allow specifying input file as a command-line argument
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("Failed to open input file");
            return EXIT_FAILURE;
        }
        printf("Input file '%s' opened successfully.\n", argv[1]);
    } else {
        yyin = fopen("testProg.cmm", "r");
        if (!yyin) {
            perror("Failed to open default input file 'testProg.cmm'");
            return EXIT_FAILURE;
        }
        printf("Input file 'testProg.cmm' opened successfully.\n");
    }

    if (yyparse() == 0) {
        printf("Parsing successful!\n");

        // Traverse the AST and perform semantic analysis
        traverseAST(root, 0);
        printSymbolTable(symbolTable);
        semanticAnalyze(root, symbolTable);
        //printASTNode(root);
        printTAC();
        printf("\n--- Semantic Analysis Is Good ---\n\n");

        generateMIPS();


    } else {
        fprintf(stderr, "Parsing failed.\n");
        fclose(yyin);
        freeSymbolTable(symbolTable);
        printf("Compiler finished with errors.\n");
        return EXIT_FAILURE;
    }

    // Clean up resources
    fclose(yyin);
    freeSymbolTable(symbolTable);
    freeAST(root);
    printf("Compiler finished successfully.\n");
    return 0;
}

// Error handling
void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
    // Terminate the compiler on syntax error
    exit(EXIT_FAILURE);
}
