STRTUP   =  LIB:astartup.obj
OBJS     =  djmount.o
LIBS     =  LIB:janus.lib+LIB:amiga.lib+LIB:lc.lib
       
CC       =  lc
CCS      =  -v -d -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  blink

djmount : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to djmount lib $(LIBS)

djmount.o :   djmount.c
   $(CC) $(CCF) djmount.c

