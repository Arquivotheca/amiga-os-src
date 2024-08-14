SU   = LIB:c.o
OBJ  = timeserv.o
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

timeserv.ld.nostrip: $(OBJ)
   bumprev 36 timeserv_rev
   $(CC) $(CCF)revision.o revision.c
   delete timeserv_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to timeserv.ld NODEBUG

.c.o :
   $(CC) $(CCF) $*

release:
   copy \#?         Other:janus/Amiga/cur/timeserv
   spat "chmod 755" Other:janus/Amiga/cur/timeserv/\#?
   delete           Other:janus/Amiga/cur/timeserv/\#?.o
   delete           Other:janus/Amiga/cur/timeserv/\#?.obj
   delete           Other:janus/Amiga/cur/timeserv/\#?.bak

clean:
   delete \#?.o
   delete \#?.obj
   delete \#?.bak

local:
   lock sys: off
   copy timeserv.ld   sys:pc/services/TimeServ
   copy timeserv.info sys:pc/services/TimeServ.info
   lock sys: on

disk:
   copy timeserv.ld   PCInstall:pc/services/TimeServ
   copy timeserv.info PCInstall:pc/services/TimeServ.info
