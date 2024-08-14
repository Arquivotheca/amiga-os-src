SRCDIR=	CDGS:src/pplibrary
SUBDIR=	ScreenSaver

CFILES=		screensaver.c
HFILES=		screensaver.h
MISCFILES=	makefile

OBJ = obj/

INCLUDE =       -iCDTV:include/ -iV37:include/ -iV37:inc.lattice -iinclude: 

CFLAGS =        -cfimsq -rr -m2 -me -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 

_all:	screensaver

screensaver:    $(OBJ)screensaver.o


.c.o:
        assign ppl: /
        lc $(CFLAGS) $*.c

.a.o:
        assign ppl: /
        asm $(AFLAGS) $*.a 

.asm.o:
        assign ppl: /
        casm -a $*.asm -o $(OBJ)$*.o $(AFLAGS) 

.pic.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:



$(OBJ)screensaver.o:    screensaver.c screensaver.h /playerprefsbase.h

# invoked by make internal in parent directory
_internal:
	copy $(CFILES)	$(SRCDIR)/$(SUBDIR) nopro
	copy $(HFILES) $(SRCDIR)/$(SUBDIR) nopro
	copy $(MISCFILES) $(SRCDIR)/$(SUBDIR) nopro

_clean:
	delete obj/\#?.o
