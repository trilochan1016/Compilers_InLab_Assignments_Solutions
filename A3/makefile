all:
	yacc -d expr.y
	lex expr.l
	gcc runprog.c

run: all
	./a.out < sample.txt

clean:
	-rm -f lex.yy.c y.tab.c y.tab.h a.out
