CXXLDFLAGS=../intermed/instance.o ../intermed/definedtypes.o

CXXFLAGS=-I../intermed

all: specgenerate

specgenerate: specgenerate.o
	$(CXX) -o specgenerate specgenerate.o $(CXXLDFLAGS)

specgenerate.o: specgenerate.cpp
	$(CXX) -c $(CXXFLAGS) specgenerate.cpp

clean:
	rm -f *.o specgenerate
