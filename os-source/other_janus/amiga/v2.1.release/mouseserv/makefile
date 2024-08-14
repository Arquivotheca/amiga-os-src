# NATIVE BUILD ONLY

SU=lib:c.o
CC=lc
xCCF=-csf -v -o
CCF=-csf -v -d4 -b0 -q500w1e -o
LN=blink
LNF=SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS=lib:lc.lib+lib:janus.lib+lib:amiga.lib+lib:debug.lib

INC=mouseserv.h
OBJ=telluser.o mouseserv.o

mouseserv.ld.nostrip: $(OBJ)
   bumprev 36 mouseserv_rev
   $(CC) $(CCF)revision.o revision.c
#   delete mouserv_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to mouseserv.ld NODEBUG
   delete mouseserv.ld.info
   delete mouseserv.ld.nostrip.info

mouseserv.o : mouseserv.c $(INC)
   $(CC) $(CCF)$@ mouseserv.c

telluser.o : telluser.c $(INC)
   $(CC) $(CCF)$@ telluser.c

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy mouseserv.ld   sys:pc/services/MouseServ
   copy mouseserv.info sys:pc/services/MouseServ.info
   lock sys: on

disk:
   copy mouseserv.ld   PCInstall:pc/services/MouseServ
   copy mouseserv.info PCInstall:pc/services/MouseServ.info

release:
   copy \#?         Other:janus/Amiga/cur/mouseserv
   spat "chmod 755" Other:janus/Amiga/cur/mouseserv/\#?
   delete           Other:janus/Amiga/cur/mouseserv/\#?.(o|obj|bak)
