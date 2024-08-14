STRTUP   =  LIB:c.o
OBJS     =  ex5.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
LN       =  blink

ex5 : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to ex5 lib $(LIBS)

ex5.o :   ex5.c
   $(CC) $(CCF) ex5.c

