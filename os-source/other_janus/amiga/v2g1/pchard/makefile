STRTUP   =  LIB:astartup.obj
OBJS     =  pchard.o
LIBS     =  LIB:janus.lib+LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
LN       =  blink

pchard : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to pchard lib $(LIBS)
   copy pchard pcinstall:pc/PCHard
   copy pchard.doc pcinstall:userdocs/PCHard.DOC

pchard.o :   pchard.c
   $(CC) $(CCF) pchard.c

