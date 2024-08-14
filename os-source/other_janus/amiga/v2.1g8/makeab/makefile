SU   = LIB:c.o
OBJ  = makeab.o
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

makeab.ld.nostrip: $(OBJ)
   bumprev 36 makeab_rev
   $(CC) $(CCF) revision.c
   delete makeab_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to makeab.ld NODEBUG
   delete makeab.ld.info
   delete makeab.ld.nostrip.info

.c.o :
   $(CC) $(CCF) $*

release:
   copy \#?         Other:janus/Amiga/cur/makeab
   spat "chmod 755" Other:janus/Amiga/cur/makeab/\#?
   delete           Other:janus/Amiga/cur/makeab/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy makeab.ld   c:MakeAB
   lock sys: on

disk:
   copy makeab.ld   PCInstall:c/MakeAB
