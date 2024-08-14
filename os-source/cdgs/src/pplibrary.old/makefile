
OBJ = obj/

INCLUDE =       -iCDTV:include/ -iV37:include/ -iV37:inc.lattice -iinclude: 

CFLAGS =        -cimsq -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES =     lib:lc.lib lib:amiga13.lib lib:debug.lib

LIBNAME =       playerprefs.library

pplibrary.ld:   $(OBJ)libent.o $(OBJ)libfunc.o \
                $(OBJ)falselib.o $(OBJ)enddata.o $(OBJ)commoncode.o $(OBJ)commonasm.o
                blink VER ERRORFILE with playerprefs_cr.lnk LIB $(LIBRARIES) MAP playerprefs.map F H L O S X PLAIN
                copy playerprefs.library playerprefs.ld.cr
                copy playerprefs.ld.cr df0:modules

pragmas: playerprefs_lib.fd playerprefs_private_lib.fd
        @echo "Remaking playerprefs_pragmas.h & playerprefs_private_pragmas.h"
        fd2pragma playerprefs_lib.fd playerprefs_pragmas.h
        fd2pragma playerprefs_private_lib.fd playerprefs_private_pragmas.h


bumprev:
         bumprev 40 playerprefs_rev

release:
         lmk bumprev
         lmk pragmas


.c.o:
        assign ppl: ""
        lc $(CFLAGS) $*.c

.a.o:
        assign ppl: ""
        asm $(AFLAGS) $*.a 

.asm.o:
        assign ppl: ""
        casm -a $*.asm -o $(OBJ)$*.o $(AFLAGS) 

.pic.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:


$(OBJ)commoncode.o:     commoncode.c playerprefsbase.h

$(OBJ)commonasm.o:      commonasm.a playerprefsbase.i

$(OBJ)libent.o:         libent.a playerprefs_rev.i playerprefsbase.i playerprefs_rev.i
                        asm -u -iCDTV:include/ -iV37:include/ -iinclude: -o$(OBJ)libent.o libent.a

$(OBJ)libfunc.o:        libfunc.a

$(OBJ)falselib.o:       falselib.a playerprefsbase.i

$(OBJ)enddata.o:        enddata.a

