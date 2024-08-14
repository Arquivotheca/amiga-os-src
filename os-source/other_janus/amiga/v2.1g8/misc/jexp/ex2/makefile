STRTUP   =  LIB:c.o
OBJS     =  ex2.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex2 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex2 lib $(LIBS)

ex2.o :   ex2.c
   $(CC) $(CCF) ex2.c

