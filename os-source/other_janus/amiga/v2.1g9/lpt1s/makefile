SU   = LIB:c.o
OBJ  =  mainlpt1.o parallel.obj
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

lpt1s.ld.nostrip: $(OBJ)
   bumprev 36 lpt1_rev
   $(CC) $(CCF) revision.c
#   delete lpt1_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to lpt1s.ld NODEBUG
   delete lpt1s.ld.info
   delete lpt1s.ld.nostrip.info

mainlpt1.o :   mainlpt1.c
   $(CC) $(CCF) mainlpt1.c

parallel.obj :   parallel.asm
	casm -a parallel.asm -y -o parallel.obj -cs -i work:lc/include

.c.o :
   $(CC) $(CCF) $*

release:
   copy \#?         Other:janus/Amiga/cur/lpt1s
   spat "chmod 755" Other:janus/Amiga/cur/lpt1s/\#?
   delete           Other:janus/Amiga/cur/lpt1s/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy lpt1s.ld   sys:pc/LPT1S
   copy lpt1s.info sys:pc/LPT1S.info
   lock sys: on

disk:
   copy lpt1s.ld   PCInstall:side/LPT1S
   copy lpt1s.info PCInstall:side/LPT1S.info
