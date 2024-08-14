
.c.o:
        lc  -d1 -O -v -cfist -o$*.o $*.c

all:    group

group : group.o
        blink group.o to $@ SC SD ND lib lib:lc.lib

release :
        copy group.c group group.h group.rev makefile envoy:src/c/group
        protect envoy:src/c/group +rewd QUIET

