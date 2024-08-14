SU   = LIB:c.o
OBJ  = dosserv.o pat.obj sendpacket.o
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

dosserv.ld.nostrip: $(OBJ)
   bumprev 36 dosserv_rev
   $(CC) $(CCF)revision.o revision.c
   delete dosserv_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to dosserv.ld NODEBUG
   delete dosserv.ld.info
   delete dosserv.ld.nostrip.info

.c.o :
   $(CC) $(CCF) $*

.asm.obj :
   casm -a $*.asm -Y -o $*.obj -cs

release:
   copy \#?         Other:janus/Amiga/cur/dosserv
   spat "chmod 755" Other:janus/Amiga/cur/dosserv/\#?
   delete           Other:janus/Amiga/cur/dosserv/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy dosserv.ld   sys:pc/services/DOSServ
   copy dosserv.info sys:pc/services/DOSServ.info
   lock sys: on

disk:
   copy dosserv.ld   PCInstall:pc/services/DOSServ
   copy dosserv.info PCInstall:pc/services/DOSServ.info
