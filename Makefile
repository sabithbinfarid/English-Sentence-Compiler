CC = gcc
CFLAGS ?= -w
CPPFLAGS ?= -Isrc
BISON ?= bison
FLEX ?= flex

.PHONY: all flexbison clean run-examples

# Default build uses the Flex/Bison pipeline.
all: flexbison

flexbison: src/parser.tab.c lex.yy.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o blockc.exe src/parser.tab.c lex.yy.c -lm

src/parser.tab.c src/parser.tab.h: src/parser.y
	$(BISON) -d -o src/parser.tab.c src/parser.y

lex.yy.c: src/lexer.l src/parser.tab.h
	$(FLEX) src/lexer.l

run-examples: flexbison
	.\blockc.exe examples\01_hello.blk --run
	.\blockc.exe examples\02_conditions.blk --run
	.\blockc.exe examples\03_loops.blk --run
	.\blockc.exe examples\04_functions.blk --run
	.\blockc.exe examples\05_full.blk --run

clean:
	del /f blockc.exe
	del /f src\parser.tab.c src\parser.tab.h lex.yy.c
	del /f examples\*.c examples\*.out examples\*.out.exe
