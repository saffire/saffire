all:
	bison -d -v -o parser.tab.c saffire.y
	lex -d saffire.l
	gcc -c parser.tab.c -o parser.tab.o
	gcc -c lex.yy.c -o lex.yy.o -lfl
	gcc -c main.c -o main.o
	gcc -o saffire main.o lex.yy.o parser.tab.o
	./saffire test.sf

clean:
	@rm parser.tab.c parser.tab.h lex.yy.c *.o
