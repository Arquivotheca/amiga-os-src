#
# Makefile for test routines
#

.c.o: 
	lc -i//include -ccusf -v $*

LIB	= lib:lc.lib
OBJ	= lance-test.o ffs.o inet_up.o
LHDR = lance.h version.h  

Lance-Test: $(OBJ) Makefile	
	Blink lib:c.o $(OBJ) to Lance-Test lib $(LIB) sc sd batch map lance.map hsxol 

lance-test.o:	lance-test.c $(LHDR) 

ffs.o:		ffs.c

inet_up.o:	inet_up.c

