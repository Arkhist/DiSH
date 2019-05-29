CC=clang
CFLAGS=-Wall -Wextra -std=c11 -Wno-implicit-function-declaration
INCLUDES=-I./include -I./parser
LDFLAGS=-lfl
CFLAGS+= $(INCLUDES)

LEXGEN=flex
PARGEN=bison

DEBUG=no
ifeq ($(DEBUG), yes)
	CFLAGS+= -g -O0
else
	CFLAGS+= -O3
endif

OBJ=$(SRC:.c=.o)
SRC=src/main.c\
	parser/parser.c\
	parser/lexer.c\
	src/common.c\
	src/shell.c\
	src/internal.c\
	src/environment.c
NAME=dish

 

default: all

all: dish
re: clean dish

dish: $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(CFLAGS) $(INCLUDES) $(LDFLAGS)

parser/parser.o: parser/parser.y parser/lexer.l
	$(PARGEN) parser/parser.y --output=parser/parser.c --defines=parser/parser.h &&\
	$(LEXGEN) --outfile=parser/lexer.c -s parser/lexer.l &&\
	$(CC) -o parser/lexer.o parser/lexer.c $(CFLAGS) $(INCLUDES) -c &&\
	$(CC) -o parser/parser.o parser/parser.c $(CFLAGS) $(INCLUDES) -c



.PHONY: clean cleanlog cleanobj cleanallobj
clean: cleanobj
	rm -f dish

cleanobj:
	rm -f $(OBJ);
	rm -f parser/lexer.o parser/lexer.c parser/parser.c parser/parser.h