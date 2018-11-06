all: parser

parser: driver.o lex.yy.o datadecl.tab.o instance.o
	$(CXX) -o parser  driver.o instance.o lex.yy.o datadecl.tab.o

driver.o: driver.cpp datadecl.tab.h instance.h
	$(CXX) -c driver.cpp

datadecl.tab.h: datadecl.tab.c

datadecl.tab.o: datadecl.tab.c
	$(CXX) -c datadecl.tab.c

datadecl.tab.c:  datadecl.y
	bison --defines datadecl.y

lex.yy.o: lex.yy.c
	$(CXX) -c lex.yy.c


lex.yy.c: datadecl.l instance.h
	flex datadecl.l

instance.o: instance.cpp instance.h
	$(CXX) -c instance.cpp

clean:
	rm -f *.o datadecl.tab.h *.c

