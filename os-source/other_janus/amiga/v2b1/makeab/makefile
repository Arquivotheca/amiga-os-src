STRTUP   =  LIB:c.o
OBJS     =  makeab.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib
       
CC       =  lc
CCF      =  -v -dDEBUG=1
CCS      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

makeab : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to makeab lib $(LIBS)
   copy makeab pcinstall:+c/MakeAB
   copy makeab.doc pcinstall:docs/user/MakeAB.DOC

makeab.o :   makeab.c
   $(CC) $(CCF) makeab.c

