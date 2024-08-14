STRTUP   =  LIB:c.o
OBJS     =  ex7.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex7 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex7 lib $(LIBS)

ex7.o :   ex7.c
   $(CC) $(CCF) ex7.c

