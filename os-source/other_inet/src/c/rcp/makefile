# ---------------------------------------------------------------------------------
# makefile for RCP
#
# $Locker:  $
#
# $Id: makefile,v 1.4 93/12/13 17:24:31 kcd Exp $
#
# $Revision: 1.4 $
#
# $Header: HOG:Other/inet/src/c/rcp/RCS/makefile,v 1.4 93/12/13 17:24:31 kcd Exp $
#
#-----------------------------------------------------------------------------------

NETBIN  = NETBIN:c
NETINC  = NETINC:
CLIB    = lib:lcr.lib lib:amiga.lib lib:debug.lib
BIN     = inet:c
OBJ     = rcp.o asyncio.o
AOBJ    = replace.o

all:    rcp

replace.o : replace.asm rcp_rev.i
rcp.o : rcp.c rcp_protos.h
asyncio.o : asyncio.h

$(AOBJ) :
        casm -a $*.asm -cqy -iinc: -o$*.o

.c.o  :
        lc -. -rr -b0 -d1 -inetinc: -cist -v -O $*.c

rcp:    $(OBJ) $(AOBJ)
        blink $(OBJ) $(AOBJ) to rcp lib $(CLIB) MAP rcp.map S X F H
        copy rcp inet:c/rcp

clean:
        delete #?.o

install:        rcp
        copy rcp $(BIN) clone
        copy rcp $(NETBIN) clone

bump:
        co -l rcp_rev.rev
        bumprev 37 rcp_rev
        ci -u -m"" rcp_rev.rev

release:
        -delete hog:Other/inet/src/c/rcp/rcp.c
        -delete hog:Other/inet/src/c/rcp/rcp_rev.i
        -delete hog:Other/inet/src/c/rcp/rcp_rev.rev
        -delete hog:Other/inet/src/c/rcp/makefile
        -delete hog:Other/inet/src/c/rcp/replace.asm
        -delete hog:Other/inet/src/c/rcp/rcp_protos.h
        -delete hog:Other/inet/src/c/rcp/asyncio.c
        -delete hog:Other/inet/src/c/rcp/asyncio.h
        copy rcp rcp.c rcp_rev.i rcp_rev.rev makefile replace.asm rcp_protos.h asyncio.c asyncio.h HOG:Other/inet/src/c/rcp
        delete hog:Other/inet/bin/c/rcp
        copy rcp hog:Other/inet/bin/c/rcp


