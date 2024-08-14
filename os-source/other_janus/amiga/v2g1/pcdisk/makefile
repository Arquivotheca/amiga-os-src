STRTUP   =  LIB:astartup.obj
OBJS     =  start.o maintask.o
LIBS     =  LIB:janus.lib LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

pcdisk : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to pcdisk lib $(LIBS)
   copy pcdisk pcinstall:pc/PCDisk
   copy pcdisk.doc pcinstall:docs/developer/amiga/PCDisk.DOC

start.o :   start.c
   $(CC) $(CCF) start.c

maintask.o :  maintask.c
   $(CC) $(CCF) maintask.c
