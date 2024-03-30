TARGET = parser

FLEX = flex
BISON = bison
RM = rm -f

CHDRS = $(wildcard ../../include/*.h)

parser: scanner.lex parser.yy
	@echo "compiling..."
	$(FLEX) -o scanner.cpp --header-file=scanner.hpp scanner.lex
	$(BISON) -v -d -o parser.cpp parser.yy

.PHONY: clean

clean:
	$(RM) *.hh scanner.hpp parser.hpp *.output parser.cpp scanner.cpp
