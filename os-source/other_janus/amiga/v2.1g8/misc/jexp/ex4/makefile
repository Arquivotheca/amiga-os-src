STRTUP   =  LIB:c.o
OBJS     =  ex4.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex4 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex4 lib $(LIBS)

ex4.o :   ex4.c
   $(CC) $(CCF) ex4.c

