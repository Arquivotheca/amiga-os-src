# NATIVE BUILD ONLY

SU    =
CC    = lc
CCF   = -cafstk -q500w1e -b0
xCCF   =-cafst -v -b0 -O -ms -rr -d -q500w1e
xAS  = assem
AS  = asm
xASF = -c w200000 -i fh1:languages/latticec5.02/lc/include -o
xASF = -c w200000 -i v36:include -o
ASF = -iv36:include/
LN    =blink
LNF   = NODEBUG SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS  = lib:lc.lib+lib:amiga.lib+lib:debug.lib

SRC=cfunctions.c devreq.c  exdir.c  gadgets.c getdisks.c global.c images.c \
    init.c       install.c prompt.c status.c

OBJ=cfunctions.o devreq.o  exdir.o  gadgets.o getdisks.o global.o images.o \
    init.o       install.o prompt.o status.o


.asm.o:
   $(AS) $(ASF) $>.asm

.c.o:
   $(CC) $(CCF) $>

install.ld: $(OBJ) makefile
   $(LN) WITH install.with TO install.ld LIBRARY $(LIBS) $(LNF)

release:
   copy \#?          v36:src/janus/install
   spat "chmod 775"  v36:src/janus/install/\#?
   delete            v36:src/janus/install/\#?.o
