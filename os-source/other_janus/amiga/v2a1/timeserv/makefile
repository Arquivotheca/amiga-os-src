OBJS     =  timeserv.o
       
CC       =  lc
CCS      =  -dDEBUG=1
CCF      =
LN       =  blink

timeserv : timeserv.o
   $(LN) with timeserv.lnk

timeserv.o :   timeserv.c timeserv.h
   $(CC) $(CCF) timeserv.c

