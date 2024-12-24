all:
	yacc -d expr.y
	lex expr.l
	gcc -o icgen intcodegen.c

run: all
	./icgen < sample.txt
	gcc -o ic intcode.c
	./ic

clean:
	-rm -f lex.yy.c y.tab.c y.tab.h intcode.c icgen ic
