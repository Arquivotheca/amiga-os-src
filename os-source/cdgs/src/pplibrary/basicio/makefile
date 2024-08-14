SRCDIR=	CDGS:src/pplibrary
SUBDIR=	BasicIO

CFILES= clicker.c libinit.c joy.c screen.c amigafunc.c bookmark.c
AFILES=	routines.a viewmgr.a
HFILES= today.h joymouse.h keyclick.h viewmgr.h
IFILES= viewmgr.i
MISCFILES= makefile

OBJ = obj/

INCLUDE =       -iinclude: -iCDTV:include/

CFLAGS =        -cfimsq -rr -m2 -me -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -o$(OBJ) -iinclude: -iCDTV:include/

_all:	basicio

basicio:        $(OBJ)amigafunc.o $(OBJ)bookmark.o $(OBJ)clicker.o $(OBJ)joy.o $(OBJ)libinit.o $(OBJ)screen.o \
                $(OBJ)routines.o $(OBJ)viewmgr.o \


.c.o:
        assign ppl: /
        lc $(CFLAGS) $*.c

.a.o:
        assign ppl: /
        asm $(AFLAGS) $*.a 

.asm.o:
        assign ppl: /
        casm -a $*.asm -o $(OBJ)$*.o $(AFLAGS) 

$(OBJ)amigafunc.o:      amigafunc.c

$(OBJ)bookmark.o:       bookmark.c /playerprefsbase.h /screensaver/screensaver.h keyclick.h

$(OBJ)clicker.o:        clicker.c /playerprefsbase.h keyclick.h

$(OBJ)joy.o:            joy.c

$(OBJ)libinit.o:        libinit.c /playerprefsbase.h /playerprefs_rev.h
                        lc $(CFLAGS) -b0 libinit.c

$(OBJ)screen.o:         screen.c /playerprefsbase.h viewmgr.h /screensaver/screensaver.h

$(OBJ)routines.o:       routines.a /playerprefs.i

$(OBJ)viewmgr.o:        viewmgr.a viewmgr.i /playerprefs_func.i

# invoked by make internal in parent directory
_internal:
	copy $(CFILES) $(SRCDIR)/$(SUBDIR) nopro
	copy $(HFILES) $(SRCDIR)/$(SUBDIR) nopro
	copy $(AFILES) $(SRCDIR)/$(SUBDIR) nopro
	copy $(IFILES) $(SRCDIR)/$(SUBDIR) nopro
	copy $(MISCFILES) $(SRCDIR)/$(SUBDIR) nopro

_clean:
	delete obj/\#?.o
