
CFLAGS= -b0

OFILES= copdis.o main.o

MYLIBS= lib:lc.lib lib:amiga.lib

.c.o:	$(OFILES)
	lc $(CFLAGS) $*

copdis: $(OFILES)
	blink <WITH <
	FROM lib:c.o $(OFILES) LIB $(MYLIBS) to copdis
	$(LFLAGS)
	<
