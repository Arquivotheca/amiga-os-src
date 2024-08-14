SU   = LIB:c.o
OBJ  = pcreset.o
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

pcreset.ld.nostrip: $(OBJ)
   bumprev 36 pcreset_rev
   $(CC) $(CCF) revision.c
   delete pcreset_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to pcreset.ld NODEBUG
   delete pcreset.ld.info
   delete pcreset.ld.nostrip.info

.c.o :
   $(CC) $(CCF) $*

release:
   copy \#?         Other:janus/Amiga/cur/pcreset
   spat "chmod 755" Other:janus/Amiga/cur/pcreset/\#?
   delete           Other:janus/Amiga/cur/pcreset/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy pcreset.ld   sys:pc/pcreset
   copy pcreset.info sys:pc/pcreset.info
   lock sys: on

disk:
   copy pcreset.ld   PCInstall:pc/pcreset
   copy pcreset.info PCInstall:pc/pcreset.info
