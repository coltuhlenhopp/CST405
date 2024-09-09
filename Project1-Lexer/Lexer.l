%option noyywrap

%{
#include <stdio.h>
#include <string.h>

#define YY_DECL int yylex()

int words = 0;
int chars = 0;
int lines = 0;

%}

letter      [a-zA-Z]
digit       [0-9]
ID          {letter}({letter}|{digit})*
NUMBER      {digit}+(\.{digit}+)?(E[+\-]?(digit)+)?
STRING      \"([^\\\"]|\\.)*\"
ws          [ \t\n]+

%%
"/*"                    {
                            int c;
                            while((c = input()) != 0) {
                                if(c == '*') {
                                    if((c = input()) == '/')
                                        break;
                                    else
                                        unput(c);
                                }
                            }
                        }
                        
"int"   {words++; chars += strlen(yytext);
         printf("%s : TYPE\n", yytext);
        }

"char"  {words++; chars += strlen(yytext);
         printf("%s : TYPE\n", yytext);
        }
        
"return" {words++; 
         chars += strlen(yytext);
         printf("%s : KEYWORD\n", yytext);
        }
        
"printf" {words++; 
         chars += strlen(yytext);
         printf("%s : FUNCTION\n", yytext);
        }
        
{ID}    {words++; chars += strlen(yytext);
         printf("%s : IDENTIFIER\n",yytext);
        }
            
{NUMBER}    {words++; chars += strlen(yytext);
              printf("%s : NUMBER\n",yytext);
            }
            
{STRING}    {words++; chars += strlen(yytext);
              printf("%s : STRING\n",yytext);
            }
            
";"     {chars++;
         printf("%s : SEMICOLON\n", yytext);
        }
        
"="     {chars++;
         printf("%s : Equal\n", yytext);
        }

"=="    {chars++;
         printf("%s : EQEQ\n", yytext);
        }

"!="    {chars++;
         printf("%s : Not Equal\n", yytext);
        }

"<"     {chars++;
         printf("%s : Less Than\n", yytext);
        }

"<="    {chars++;
         printf("%s : Less Than or Equal to\n", yytext);
        }

">"     {chars++;
         printf("%s : Greater Than\n", yytext);
        }

">="    {chars++;
         printf("%s : Greater Than or Equal To\n", yytext);
        }

"+"     { 
        chars++; 
        printf("%s : PLUS\n", yytext); 
        }

"-"     { 
        chars++; 
        printf("%s : MINUS\n", yytext); 
        }
        
"*"     { 
        chars++; 
        printf("%s : TIMES\n", yytext); 
        }

"/"     { 
        chars++; 
        printf("%s : DIVIDE\n", yytext); 
        }

"("     { 
        chars++; 
        printf("%s : LPAREN Delimiter\n", yytext); 
        }

")"     { 
        chars++; 
        printf("%s : RPAREN Delimiter\n", yytext); 
        }

"{"     { 
        chars++; 
        printf("%s : LBRACE Delimiter\n", yytext); 
        }

"}"     { 
        chars++; 
        printf("%s : RBRACE Delimiter\n", yytext); 
        }

\"      { 
        chars++; 
        printf("%s : QUOTE\n", yytext); 
        }
        
\n      {lines++; chars=0;}
[ \t]+  {chars++;}  /* Handles whitespace and increments char count */
.       {chars++;
         printf("%s : Unrecognized symbol at line %d char %d\n", yytext,lines,chars);
        }
        

%%
int main(int argc, char**argv)
{
    #ifdef YYDEBUG
        yydebug = 1;
    #endif

    printf("Compiler started. \n\n");

    if (argc > 1){
      if(!(yyin = fopen(argv[1], "r")))
          {
        perror(argv[1]);
        return(1);
      }
    }
    yylex();
}
