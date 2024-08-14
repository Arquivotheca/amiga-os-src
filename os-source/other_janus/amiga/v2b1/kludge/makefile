STRTUP   =  LIB:c.o
OBJS     =  loadabs.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

loadabs : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to loadabs lib $(LIBS)

loadabs.o :   loadabs.c
   $(CC) $(CCF) loadabs.c

