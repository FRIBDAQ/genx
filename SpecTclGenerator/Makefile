CXXLDFLAGS=../intermed/instance.o ../intermed/definedtypes.o

CXXFLAGS=-I../intermed

all: specgenerate

install: specgenerate
	install -d $(PREFIX)/bin
	install specgenerate $(PREFIX)/bin


specgenerate: specgenerate.o
	$(CXX) -o specgenerate specgenerate.o $(CXXLDFLAGS)

specgenerate.o: specgenerate.cpp
	$(CXX) -c $(CXXFLAGS) specgenerate.cpp

clean:
	rm -f *.o specgenerate
