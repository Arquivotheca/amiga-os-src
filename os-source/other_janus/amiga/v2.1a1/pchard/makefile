SU   = LIB:c.o
OBJ  = pchard.o
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

pchard.ld.nostrip: $(OBJ)
   bumprev 36 pchard_rev
   $(CC) $(CCF) revision.c
   delete pchard_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to pchard.ld NODEBUG
   delete pchard.ld.info
   delete pchard.ld.nostrip.info

.c.o :
   $(CC) $(CCF) $*

release:
   copy \#?         Other:janus/Amiga/cur/pchard
   spat "chmod 755" Other:janus/Amiga/cur/pchard/\#?
   delete           Other:janus/Amiga/cur/pchard/\#?.o
   delete           Other:janus/Amiga/cur/pchard/\#?.obj
   delete           Other:janus/Amiga/cur/pchard/\#?.bak

clean:
   delete \#?.o
   delete \#?.obj
   delete \#?.bak

local:
   lock sys: off
   copy pchard.ld   sys:pc/PCHard
   copy pchard.info sys:pc/PCHard.info
   lock sys: on

disk:
   copy pchard.ld   PCInstall:pc/PCHard
   copy pchard.info PCInstall:pc/PCHard.info
