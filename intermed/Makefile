all: parser

parser: driver.o lex.yy.o datadecl.tab.o
	$(CXX) -o parser  driver.o lex.yy.o datadecl.tab.o

driver.o: driver.cpp datadecl.tab.h
	$(CXX) -c driver.cpp

datadecl.tab.h: datadecl.tab.c

datadecl.tab.o: datadecl.tab.c
	$(CXX) -c datadecl.tab.c

datadecl.tab.c:  datadecl.y
	bison --defines datadecl.y

lex.yy.o: lex.yy.c
	$(CXX) -c lex.yy.c


lex.yy.c: datadecl.l
	flex datadecl.l

clean:
	rm -f *.o datadecl.tab.h *.c

