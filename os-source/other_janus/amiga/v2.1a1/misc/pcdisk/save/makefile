STRTUP   =  LIB:c.o
OBJS     =  start.o maintask.o
LIBS     =  lib:lc.lib+LIB:janus.lib LIB:amiga.lib+lib:debug.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink
xLNF      =
LNF     = define __main=__tinymain

pcdisk : $(OBJS)
   $(LN) $(LNF) from $(STRTUP) $(OBJS) to pcdisk lib $(LIBS)

start.o :   start.c
   $(CC) $(CCF) start.c

maintask.o :  maintask.c
   $(CC) $(CCF) maintask.c
