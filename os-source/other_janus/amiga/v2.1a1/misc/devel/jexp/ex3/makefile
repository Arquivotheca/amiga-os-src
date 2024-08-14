STRTUP   =  LIB:c.o
OBJS     =  ex3.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex3 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex3 lib $(LIBS)

ex3.o :   ex3.c
   $(CC) $(CCF) ex3.c

