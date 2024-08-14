#
# Makefile for query
#

NETLIB	= net32.lib
CLIB	= c32.lib
BIN	= work:nbin

.c.o:
	cc -DAMIGA +l -I///include $*.c

all:	query trace

query:	v_query.o
	ln -m +q -o $@ v_query.o $@.o $(NETLIB) $(CLIB)

trace:	v_trace.o
	ln -m +q -o $@ v_trace.o $@.o $(NETLIB) $(CLIB)

v_query.o: query.o
	vers >v_query.c -v query
	cc v_query.c

v_trace.o: trace.o
	vers >v_trace.c -v trace
	cc v_trace.c

clean:
	delete #?.o

install:	query trace
	copy query $(BIN)
	copy trace $(BIN)
