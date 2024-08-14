
# makefile for lmk

INCLDS =  cdtv_cr_hw.i cddev.i cdtv_rev.i cdtv.i
OBJ = obj/
LST = lst/

cdtv.ld: $(OBJ)dev.o $(OBJ)cmd.o $(OBJ)print.o
       blink with cdtv.lnk ND
       copy cdtv.ld cdtv.device
       copy cdtv.ld df0:modules
       copy cdtv.device df0:devs


bumprev:
         bumprev 40 cdtv_rev

release:
         smake bumprev
         smake

.asm.o:
        hx68 -a $*.asm -o $(OBJ)$*.o -s


$(OBJ)dev.o: dev.asm     $(INCLDS)

$(OBJ)cmd.o: cmd.asm     $(INCLDS)

$(OBJ)print.o: print.asm $(INCLDS)


