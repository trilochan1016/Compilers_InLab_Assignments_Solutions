all:
	lex exprlist.l
	gcc evalexpr.c -ll

run: ./a.out
	./a.out < list.txt
	./a.out < biglist.txt
	-./a.out < errlist1.txt
	-./a.out < errlist2.txt
	-./a.out < errlist3.txt
	-./a.out < errlist4.txt
	-./a.out < errlist5.txt
	-./a.out < errlist6.txt

clean:
	-rm -f a.out lex.yy.c
