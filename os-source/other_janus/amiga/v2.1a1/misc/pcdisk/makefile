STRTUP   =  LIB:c.o
OBJS     =  start.o maintask.o dprintf.o
LIBS     =  lib:lc.lib+LIB:janus.lib LIB:amiga.lib+lib:debug.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v -d4
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink
xLNF     =
LNF      = define __main=__tinymain

pcdisk.ld.nostrip : $(OBJS)
   $(LN) $(LNF) from $(STRTUP) $(OBJS) to $@ lib $(LIBS)
   $(LN) $@ to pcdisk.ld NODEBUG

start.o :   start.c
   $(CC) $(CCF) start.c

maintask.o :  maintask.c
   $(CC) $(CCF) maintask.c

dprintf.o :  dprintf.c
   $(CC) $(CCF) dprintf.c

release:
   SPAT "ci -l" \#?.c
   SPAT "ci -l" \#?.a
   SPAT "ci -l" \#?.asm
   SPAT "ci -l" \#?.h
   SPAT "ci -l" \#?.i
   SPAT "ci -l" \#?.doc
   SPAT "ci -l" Makefile
