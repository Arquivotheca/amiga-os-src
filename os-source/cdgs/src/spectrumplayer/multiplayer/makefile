SRCDIR= CDGS:src/pplibrary
SUBDIR= Player

OBJ = obj/

INCLUDE =       IDIR=include: IDIR=cdgs:include/

CFLAGS =        NMINC MCCONS STRMERGE STREQ PARMS=REG CPU=68020 OPTSIZE NOVERSION OPTIMIZE NOSTKCHK DEBUG=L OBJNAME=$(OBJ) $(INCLUDE)

AFLAGS =        -o$(OBJ) -iinclude: 

CFILES=         player.c

LIBRARIES =     lib:sc.lib lib:amiga.lib lib:debug.lib

ASSEMBLER         = HX68 -a
ASSEMBLER_OPTS    = -csy -i include: -o

_all:   player

player:         $(OBJ)init.o $(OBJ)player.o $(OBJ)display.o $(OBJ)misc.o $(OBJ)animcontrol.o\
                $(OBJ)screendata.o $(OBJ)buttondata.o $(OBJ)cdgdata.o
                slink FROM lib:c.o $(OBJ)init.o $(OBJ)player.o $(OBJ)display.o $(OBJ)misc.o $(OBJ)animcontrol.o \
                           $(OBJ)screendata.o $(OBJ)buttondata.o $(OBJ)cdgdata.o \
                        TO Player LIB $(LIBRARIES) ND
                copy player df0:

.c.o:
        assign ppl: /
        sc $(CFLAGS) $*.c

.asm.o:
    $(ASSEMBLER) $*.asm $(ASSEMBLER_OPTS) $(OBJ)$*.o

.pic.o:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x



$(OBJ)init.o:           init.c          player.h keydefs.h

$(OBJ)player.o:         player.c        player.h keydefs.h

$(OBJ)display.o:        display.c       player.h keydefs.h

$(OBJ)misc.o:           misc.asm        

$(OBJ)animcontrol.o:    animcontrol.asm 

$(OBJ)screendata.o:     screendata.pic  

$(OBJ)buttondata.o:     buttondata.pic  

$(OBJ)cdgdata.o:        cdgdata.pic     
