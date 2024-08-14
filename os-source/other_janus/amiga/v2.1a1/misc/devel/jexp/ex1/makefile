STRTUP   =  LIB:c.o
OBJS     =  ex1.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex1 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex1 lib $(LIBS)

ex1.o :   ex1.c
   $(CC) $(CCF) ex1.c

