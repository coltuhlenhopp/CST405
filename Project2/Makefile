# Makefile for mini compiler project

# Target to build the parser
all: parser

# Bison target to generate parser files
mini_parser1.tab.c mini_parser1.tab.h: mini_parser1.y
	bison -t -v -d mini_parser1.y

# Flex target to generate lexer file
lex.yy.c: mini_lexer.l mini_parser1.tab.h
	flex mini_lexer.l

# Target to compile the parser and related files
parser: lex.yy.c mini_parser1.tab.c mini_parser1.tab.h AST1.c symbol_table.c
	gcc -o parser mini_parser1.tab.c lex.yy.c AST1.c symbol_table.c

# Target to run the parser with input file
run: parser
	./parser testProg.cmm

# Clean target to remove generated files
clean:
	rm -f parser mini_parser1.tab.c lex.yy.c mini_parser1.tab.h mini_parser1.output lex.yy.o mini_parser1.tab.o AST1.o symbol_table.o
	ls -l
