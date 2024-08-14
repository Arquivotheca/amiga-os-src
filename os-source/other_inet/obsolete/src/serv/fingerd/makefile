#
# Makefile for fingerd
#

NETLIB	= NETBIN:lib/socket32.lib
CLIB	= c32.lib
BIN	= NETBIN:serv
OBJ	= fingerd.o

.c.o:
	cc -DAMIGA +l -Inetinc: $*.c

all:	fingerd

fingerd:	version.o 
	ln -m +q -o $@ version.o $(OBJ) $(NETLIB) $(CLIB)

version.o: $(OBJ)
	vers >version.c -v fingerd
	cc version.c

clean:
	delete #?.o

install:	fingerd
	copy fingerd $(BIN) clone
