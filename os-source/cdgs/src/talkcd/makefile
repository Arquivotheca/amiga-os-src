
OBJ = obj/

INCLUDE = IDIR=V39:include/ IDIR=include:

CFLAGS =  NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE DEBUG=L GST t:all.gst OBJNAME=$(OBJ) $(INCLUDE)
AFLAGS =        -icdgs:include/ -iV39:include/ -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM
FLAGS  =  -s -i include:

LIBRARIES =     lib:sc.lib lib:amiga.lib lib:debug.lib


TalkCD.ld:   $(OBJ)TalkCD.o $(OBJ)interrupt.o
    slink FROM lib:c.o $(OBJ)TalkCD.o $(OBJ)interrupt.o to TalkCD.ld LIB $(LIBRARIES) ND
    copy TalkCD.ld TalkCD

.asm.o:
        hx68 -a $*.asm -o $(OBJ)$*.o $(FLAGS)

.c.o:
    sc $(CFLAGS) $*

bumprev:
        bumprev 40 TalkCD_rev

$(OBJ)TalkCD.o:         TalkCD.c

$(OBJ)interrupt.o:      interrupt.asm
