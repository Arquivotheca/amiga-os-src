SU   = LIB:c.o
OBJ  =  lpt1.o
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

lpt1.ld.nostrip: $(OBJ)
   bumprev 36 lpt1_rev
   $(CC) $(CCF) revision.c
#   delete lpt1_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to lpt1.ld NODEBUG
   delete lpt1.ld.info
   delete lpt1.ld.nostrip.info

lpt1.o :   lpt1.c
   $(CC) $(CCF) lpt1.c

.c.o :
   $(CC) $(CCF) $*

release:
   copy \#?         Other:janus/Amiga/cur/lpt1
   spat "chmod 755" Other:janus/Amiga/cur/lpt1/\#?
   delete           Other:janus/Amiga/cur/lpt1/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy lpt1.ld   sys:pc/LPT1
   copy lpt1.info sys:pc/LPT1.info
   lock sys: on

disk:
   copy lpt1.ld   PCInstall:pc/LPT1
   copy lpt1.info PCInstall:pc/LPT1.info
