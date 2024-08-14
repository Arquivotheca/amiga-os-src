STRTUP   =  LIB:astartup.obj
OBJS     =  pchard.o
LIBS     =  LIB:janus.lib+LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

pchard : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to pchard lib $(LIBS)
   copy pchard pcinstall:pc/pchard
   copy pchard.doc pcinstall:docs/user/PCHard.DOC

pchard.o :   pchard.c
   $(CC) $(CCF) pchard.c

