all: parser desertest

install: parser
	install -d $(PREFIX)/bin
	install parser $(PREFIX)/bin

parser: driver.o lex.yy.o datadecl.tab.o instance.o definedtypes.o
	$(CXX) -o parser  driver.o instance.o definedtypes.o lex.yy.o datadecl.tab.o

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

definedtypes.o: definedtypes.cpp definedtypes.h instance.h
	$(CXX) -c definedtypes.cpp

desertest: deserializetest.o definedtypes.o instance.o
	$(CXX) -o desertest deserializetest.o definedtypes.o instance.o

deserializetest.o: deserializetest.cpp instance.h definedtypes.h


clean:
	rm -f parser desertest *.o datadecl.tab.h *.c

