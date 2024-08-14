#
# makefile for routed
#

NETLIB	= NETBIN:lib/socket32.lib
CLIB	= c32.lib
BIN	= NETBIN:serv

.c.o:
	cc +m -DAMIGA +l -Inetinc: $*.c

OBJS=	af.o if.o input.o main.o output.o startup.o tables.o timer.o \
	trace.o inet.o compat.o

all: routed

routed: version.o
	ln -m +q -o $@ version.o $(CFLAGS) $(OBJS) $(NETLIB) $(CLIB)

version.o: $(OBJS)
	vers >version.c -v routed
	cc version.c

clean:
	-delete #?.o

install: routed
	copy routed $(BIN) clone
