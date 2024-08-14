STRTUP   =  LIB:c.o
OBJS     =  makeab.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

makeab : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to makeab lib $(LIBS)
   copy makeab pcinstall:+c/MakeAB
   copy makeab.doc pcinstall:userdocs/MakeAB.DOC

makeab.o :   makeab.c
   $(CC) $(CCF) makeab.c

