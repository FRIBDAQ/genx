CXXLDFLAGS=../intermed/instance.o ../intermed/definedtypes.o
CXXFLAGS=-I../intermed -std=c++11

all: rootgenerate

install: rootgenerate
	install -d $(PREFIX)/bin
	install rootgenerate $(PREFIX)/bin


rootgenerate: rootgenerate.o
	$(CXX) -o rootgenerate rootgenerate.o $(CXXLDFLAGS)


rootgenerate.o: rootgenerate.cpp
	$(CXX) -c $(CXXFLAGS) rootgenerate.cpp

clean:
	rm -f *.o rootgenerate
