genx: genx.o genxparams.o
	$(CXX) -o genx genx.o genxparams.o

genx.o: genx.cpp genxparams.h
	$(CXX) -c genx.cpp -DPREFIX=$(PREFIX)

genxparams.o: genxparams.c
	$(CC) -c genxparams.c

genxparams.h: genxparams.c

genxparams.c: genxparams.ggo
	gengetopt <genxparams.ggo -Fgenxparams

install: genx
	install -d $(PREFIX)/bin
	install genx $(PREFIX)/bin 
