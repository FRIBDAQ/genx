CXXLDFLAGS=../intermed/instance.o ../intermed/definedtypes.o
CXXFLAGS=-I../intermed

all: rootgenerate

rootgenerate: rootgenerate.o
	$(CXX) -o rootgenerate rootgenerate.o $(CXXLDFLAGS)


rootgenerate.o: rootgenerate.cpp
	$(CXX) -c $(CXXFLAGS) rootgenerate.cpp

clean:
	rm -f *.o rootgenerate
