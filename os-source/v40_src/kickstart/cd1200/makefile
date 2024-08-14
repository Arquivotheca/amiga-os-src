
# makefile for smake

#
# Server directory for the data...
#
REMOTE = V:src/kickstart/cd

INCLDS =  cdprivate.i cd_rev.i cd.i cdgs_hw.i
FLAGS  =  -s -i include:
OBJ = obj/
LST = lst/

cd.ld: $(OBJ)main.o $(OBJ)task.o $(OBJ)functions.o $(OBJ)drive.o $(OBJ)interrupt.o $(OBJ)read.o $(OBJ)ecc.o $(OBJ)print.o
       blink obj/main.o obj/task.o obj/drive.o obj/interrupt.o obj/read.o obj/ecc.o obj/functions.o obj/print.o \
             LIBRARY lib:amiga.lib TO cd.ld VERBOSE SMALLCODE SMALLDATA addsym
       blink cd.ld to cd.ld.strip ND quiet
       copy cd.ld.strip cd.device


autodoc:
        autodoc >cd.doc main.asm task.asm

#
# Release the current build to the server
#
internal:
    @delete $(REMOTE)/((\#?.(c|h|asm|i|ld|ld.strip|rev|doc|sfd))|makefile|lmkfile)
    @protect ((\#?.(c|h|asm|i|ld|ld.strip|rev|doc|sfd))|makefile|lmkfile) +r quiet
    @copy ((\#?.(c|h|asm|i|ld|ld.strip|rev|doc|sfd))|makefile|lmkfile) $(REMOTE)
    @copy \#?.doc V:doc NOPRO
    @copy \#?.sfd V:lib/sfd NOPRO
    @copy cd.? v:include/devices NOPRO
    @copy cdprivate.i v:include/internal NOPRO

.asm.o:
        hx68 -a $*.asm -o $(OBJ)$*.o $(FLAGS)

bumprev:
         bumprev 40 cd_rev

release:
         smake bumprev
         smake
         smake autodoc



$(OBJ)main.o: main.asm                                         $(INCLDS)

$(OBJ)task.o: task.asm                                         $(INCLDS)

$(OBJ)functions.o: functions.asm                               $(INCLDS)

$(OBJ)drive.o: drive.asm                                       $(INCLDS)

$(OBJ)interrupt.o: interrupt.asm                               $(INCLDS)

$(OBJ)read.o: read.asm                                         $(INCLDS)

$(OBJ)ecc.o: ecc.asm                                           $(INCLDS)

$(OBJ)print.o: print.asm                                       $(INCLDS)


