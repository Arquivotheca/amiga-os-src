
.c.o:
        lc  -d1 -O -v -cfist -o$*.o $*.c

all:    owner

owner : owner.o
        blink owner.o to $@ SC SD ND lib lib:lc.lib

release :
        copy owner.c owner.rev owner.h owner makefile envoy:src/c/owner
        protect envoy:src/c/owner/#? +rewd QUIET



