
# makefile for smake

INCLDS =  internal.i freeanim_rev.i
FLAGS  =  -i include:
OBJ = obj/
LST = lst/

freeanim.ld: $(OBJ)main.o
             slink obj/main.o TO freeanim.ld SMALLCODE SMALLDATA
             slink freeanim.ld TO freeanim.ld.strip ND
             copy freeanim.ld freeanim.library
             copy freeanim.ld libs:freeanim.library
             copy freeanim.ld df0:modules/freeanim.ld
             copy freeanim.ld df0:libs/freeanim.library


.asm.o:
        hx68 -a $*.asm -o $(OBJ)$*.o $(FLAGS)


bumprev:
         bumprev 40 freeanim_rev

release:
         smake bumprev
         smake



$(OBJ)main.o: main.asm                                         $(INCLDS)

