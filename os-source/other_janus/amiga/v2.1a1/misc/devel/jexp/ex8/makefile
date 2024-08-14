STRTUP   =  LIB:c.o
OBJS     =  ex8.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex8 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex8 lib $(LIBS)

ex8.o :   ex8.c
   $(CC) $(CCF) ex8.c

