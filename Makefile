all:
	bison -d -v -o parser.tab.c saffire.y
	lex  saffire.l
	gcc -c parser.tab.c -o parser.tab.o
	gcc -c lex.yy.c -o lex.yy.o -lfl
	gcc -c main.c -o main.o
	gcc -o saffire main.o lex.yy.o parser.tab.o
	./saffire tests/001.sf
	./saffire tests/002.sf
	./saffire tests/003.sf

clean:
	@rm parser.tab.c parser.tab.h lex.yy.c *.o
