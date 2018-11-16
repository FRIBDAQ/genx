all: empty

empty:

install:
	install -d $(PREFIX)/share/html
	install -d $(PREFIX)/share/images
	docbook2html -o $(PREFIX)/share/html genx.xml
	(cd images; tar czf - .) | (cd $(PREFIX)/share/images; tar xzf - )

