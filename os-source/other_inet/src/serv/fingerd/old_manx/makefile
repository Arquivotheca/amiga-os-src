#
# Makefile for fingerd
#

NETLIB	= net32.lib
CLIB	= c32.lib
BIN	= inet:serv
OBJ	= fingerd.o

.c.o:
	cc -DAMIGA +l -I//include $*.c

all:	fingerd

fingerd:	version.o a:lib/net/net32.lib
	ln -m +q -o $@ version.o $(OBJ) $(NETLIB) $(CLIB)

version.o: $(OBJ)
	vers >version.c -v fingerd
	cc version.c

clean:
	delete #?.o

install:	fingerd
	copy fingerd $(BIN) clone
