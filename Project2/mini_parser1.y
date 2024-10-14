%{

// Colt & Teddy
// Parser for the Compiler

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "AST1.h"
#include "symbol_table.h"
#include "sem.h"
#include "optimize.h"
#define TABLE_SIZE 100

#include <ctype.h>
#include <stdbool.h>


// Create extern variables (chatgpt stuff)
extern int yylex();
extern int yyparse(void);
extern FILE* yyin;
extern int yylineno;
extern TAC* tHead;
extern struct ASTNode* root;
void yyerror(const char* s);
ASTNode* root = NULL; 
SymbolTable* symTab = NULL;
Symbol* symbol = NULL;
int parserErrorFlag = 0;

%}


%union {
    int number;
    char character;
    char* string;
    char* operators;
    struct ASTNode* ast;
}

%token <string> TYPE
%token <string> ID
%token <char> SEMICOLON
%token <operators> EQ
%token <operators> PLUS
%token <number> NUMBER
%token <string> WRITE
%token UNRECOGNIZED

%left PLUS

%printer { fprintf(yyoutput, "%s", $$); } ID;

%type <ast> Program VarDecl VarDeclList Stmt StmtList Expr
%start Program

%%

Program:
    VarDeclList StmtList {
        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem Failure in Program.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_Program;
        $$->program.varDeclList = $1;
        $$->program.stmtList = $2;
        root = $$; // Set the global root pointer to the root of the AST

    }
;

VarDeclList:{
        $$ = NULL; // Handle case where there are no variable declarations
    }
    | VarDecl VarDeclList {
        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem Failure in VarDeclList.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_VarDeclList;
        $$->varDeclList.varDecl = $1;
        $$->varDeclList.varDeclList = $2;

    }
;

VarDecl:
    TYPE ID SEMICOLON {
        if (lookupSymbol(symTab, $2) != NULL) {
            fprintf(stderr, "Variable '%s' is declared already at line %d.\n", $2, yylineno);
            parserErrorFlag = 1;
            YYABORT;
        }

        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem Failure for VarDecl.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_VarDecl;
        $$->varDecl.varType = strdup($1);
        $$->varDecl.varName = strdup($2);
        addSymbol(symTab, $2, $1, INT_MIN);

    }  
    // Check for semicolon
    | TYPE ID {
        fprintf(stderr, "Need a semicolon at line %d.\n", yylineno - 1);
        parserErrorFlag = 1;
        YYABORT;
    }
;

StmtList:
     {
        $$ = NULL; // Handle the case where there are no statements
    }
    | Stmt StmtList {
        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem failure for stmtList.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_StmtList;
        $$->stmtList.stmt = $1;
        $$->stmtList.stmtList = $2;

    }
;

Stmt:
    ID EQ Expr SEMICOLON {
        Symbol* sym = lookupSymbol(symTab, $1);
        if (sym == NULL) {
            fprintf(stderr, "Undeclared variable '%s' at line %d.\n", $1, yylineno);
            parserErrorFlag = 1;
            YYABORT;
        }

        // Store the expression result in the symbol table
        sym->intValue = evaluateExpr($3, symTab);
        char* exprResult = genExprTAC($3, symTab);   //===============================================
        genTAC("MOV", sym->name, exprResult, NULL);

        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem Failure for Stmt (assignment).\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_AssignStmt;
        $$->assignStmt.varName = strdup($1);
        $$->assignStmt.operators = $2;
        $$->assignStmt.expr = $3;

    }  
    // check for semicolon
    | ID EQ Expr  { //-------------------------------------------------------------------------
        fprintf(stderr, "Need a semicolon silly goose at line %d.\n", yylineno - 1);
        parserErrorFlag = 1;
        YYABORT;
    }
    | WRITE ID SEMICOLON {
        Symbol* sym = lookupSymbol(symTab, $2);
        if (sym == NULL) {
            fprintf(stderr, "Undeclared variable '%s' at line %d.\n", $2, yylineno);
            parserErrorFlag = 1;
            YYABORT;
        }

        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem failure for write.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_WriteStmt;
        $$->writeStmt.id = strdup($2);

        genTAC("WRITE", $$->writeStmt.id, NULL, NULL);
    }
    // Missing semicolon
    | WRITE ID  {
        fprintf(stderr, "Need a semicolon at line %d.\n", yylineno - 1);
        parserErrorFlag = 1;
        YYABORT;
    }
    | UNRECOGNIZED {
        fprintf(stderr, "Something weird at line %d.\n", yylineno);
        parserErrorFlag = 1;
        YYABORT;
    }
;

Expr:
    Expr PLUS Expr {
        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem failure for expr.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_Expr;
        $$->expr.left = $1;
        $$->expr.right = $3;
        $$->expr.operators = $2;
    }
    | ID {
        Symbol* sym = lookupSymbol(symTab, $1);
        if (sym == NULL) {
            fprintf(stderr, "Need to declare variable '%s' at line %d.\n", $1, yylineno);
            parserErrorFlag = 1;
            YYABORT;
        }

        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem failure for ID.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_SimpleID;
        $$->simpleID.name = $1;

    }
    | NUMBER {
        $$ = malloc(sizeof(ASTNode));
        if ($$ == NULL) {
            fprintf(stderr, "Mem failure for number.\n");
            exit(EXIT_FAILURE);
        }
        $$->type = NodeType_SimpleExpr;
        $$->simpleExpr.number = $1;

    }
;




%%

// Mips generator
// Function to generate MIPS code and write it to an output file
void genMIPS(TAC* tHead) {
    FILE* file = fopen("mips.asm", "w");
    if (!file) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    // Write headers to the file
    fprintf(file, ".data\n");
    fprintf(file, "newline: .asciiz \"\\n\"\n");
    fprintf(file, ".text\n");
    fprintf(file, ".globl main\n");
    fprintf(file, "main:\n");

    TAC* current = tHead;

    // Write MIPS instructions based on TAC
    while (current != NULL) {
        if (strcmp(current->operation, "MOV") == 0) {
            if (isConstant(current->oper1)) {
                fprintf(file, "    li $%s, %s\n", current->result, current->oper1); // Load immediate value into the register
            } else {
                fprintf(file, "    move $%s, $%s\n", current->result, current->oper1); // Move value between registers
            }
        } else if (strcmp(current->operation, "ADD") == 0) {
            fprintf(file, "    add $%s, $%s, $%s\n", current->result, current->oper1, current->oper2); // Add operation between registers
        } else if (strcmp(current->operation, "WRITE") == 0) {
            // MIPS syscall to print integer
            fprintf(file, "    move $a0, $%s\n", current->result); // Move value from the specified register to $a0 for printing
            fprintf(file, "    li $v0, 1\n"); // Load syscall code for print integer
            fprintf(file, "    syscall\n"); // Execute syscall
            fprintf(file, "    li $v0, 4\n"); // Load syscall code for print string
            fprintf(file, "    la $a0, newline\n"); // Load the address of the newline string into $a0
            fprintf(file, "    syscall\n"); // Execute syscall
        }
        current = current->next;
    }

    // Write exit syscall
    fprintf(file, "    li $v0, 10\n");
    fprintf(file, "    syscall");

    fclose(file);
    printf("MIPS code written to mips.asm\n");
}

//======================================================================================================================================

int main() {
    printf("Starting the compiler...\n\n");

    // Make symbol table
    symTab = createSymbolTable(TABLE_SIZE);
    printf("Symbol table created.\n\n");

    // open our simple program
    yyin = fopen("testProg.cmm", "r");
    if (!yyin){
        perror("Failure bro.\n");
        return EXIT_FAILURE;
    }

    int result = yyparse();

    //Little loop for a check
    if (result == 0){
        // semantic analyzer
        semCheck(root, symTab);

        // AST Part
        if (root != NULL){
            printf("\n -- The Greatest AST Ever Made --\n\n");
            traverseAST(root, 0);
        }

        // Symbol Table
        printSymbolTable(symTab);

        // Initial TAC
        printf("\n -- Initial TAC --\n");
        printTAC();

        // Start to optimize TAC
        replaceVariablesWithTemp(&tHead);

        // Optimizationnnnnnn
        printf("\n -- Optimized TAC --\n");
        optimizeTAC(&tHead);
        printOptimizedTAC();

        // MIPS
        printf("\n -- MIPS Code --\n");
        genMIPS(tHead);
        freeAST(root);
    } 
    else {
        fprintf(stderr, "Parsing did not work... oof\n");
    }
    freeSymbolTable(symTab);
    fclose(yyin);
    return result;
}

// Error handling
void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
    // Terminate the compiler on syntax error
    exit(EXIT_FAILURE);
}
