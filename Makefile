PREFIX=/usr/opt/genx     # Default install location.


SUBDIRS=genx intermed RootGenerator SpecTclGenerator docs

all:
	for f in $(SUBDIRS); do  (cd $$f;  make all PREFIX=$(PREFIX)); done

install:
	for f in $(SUBDIRS); do  (cd $$f;  make install  PREFIX=$(PREFIX)); done
