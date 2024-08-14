STRTUP   =  LIB:c.o
OBJS     =  ex6.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex6 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex6 lib $(LIBS)

ex6.o :   ex6.c
   $(CC) $(CCF) ex6.c

