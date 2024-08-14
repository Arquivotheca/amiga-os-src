STRTUP   =  LIB:c.o
OBJS     =  _main.o timeserv.o
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

timeserv : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to timeserv lib $(LIBS)
   copy timeserv     pcinstall:pc/TimeServ
   copy timeserv.doc pcinstall:docs/user/TimeServ.DOC
   copy timeserv.c   pcinstall:docs/developer/examples/timeserv/TimeServ.c
   copy timeserv.txt pcinstall:docs/developer/examples/timeserv/TimeServ.ReadMe
   copy makefile     pcinstall:docs/developer/examples/timeserv/MakeFile

_main.o : _main.c
   $(CC) $(CCF) _main.c

timeserv.o :   timeserv.c timeserv.h
   $(CC) $(CCF) timeserv.c

