STRTUP   =  LIB:astartup.obj
OBJS     =  start.o maintask.o
LIBS     =  LIB:janus.lib LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

pcdisk.ld : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to pcdisk.ld lib $(LIBS)

start.o :   start.c
   $(CC) $(CCF) start.c

maintask.o :  maintask.c
   $(CC) $(CCF) maintask.c
