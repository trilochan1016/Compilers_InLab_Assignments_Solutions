all:
	yacc -d poly.y
	lex poly.l
	gcc polyutil.c

run: all
	./a.out < sample.txt

clean:
	-rm -f lex.yy.c y.tab.c y.tab.h a.out
