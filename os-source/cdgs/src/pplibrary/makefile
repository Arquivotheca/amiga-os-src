SRCDIR=	CDGS:src/pplibrary
CFILES=		display.c commoncode.c
HFILES1=	playerprefs_protos.h playerprefsbase.h playerprefs.h
HFILES2=	other_protos.h display_protos.h playerprefs_private_pragmas.h
HFILES3=	display.h playerprefs_pragmas.h screencoords.h playerprefs_rev.h animate.h
AFILES=		animcontrol.a libent.a libfunc.a enddata.a
IFILES=		playerprefsbase.i playerprefs_func.i playerprefs_rev.i playerprefs.i animate.i
MISCFILES1=	makefile playerprefs_rev.rev playerprefs.ld.cr
MISCFILES2=	playerprefs_private_lib.fd playerprefs_lib.fd

OBJ=		obj/

OFILES=		obj/libent.o obj/libfunc.o obj/commoncode.o obj/display.o obj/animcontrol.o

PLAYEROFILES=	Player/obj/player.o Player/obj/gadgets.o Player/obj/cdg.o \
		Player/obj/scrdata.o Player/obj/scrbuttons.o \
		Player/obj/cdgnumb.o

SSOFILES=	ScreenSaver/obj/ScreenSaver.o

TMOFILES=	TradeMark/obj/cdtv_tm.o TradeMark/obj/cdtm.o

BASICOFILES=	BasicIO/obj/libinit.o BasicIO/obj/routines.o \
		BasicIO/obj/amigafunc.o BasicIO/obj/clicker.o \
		BasicIO/obj/screen.o BasicIO/obj/viewmgr.o \
		BasicIO/obj/bookmark.o BasicIO/obj/joy.o

LASTO=		obj/enddata.o

INCLUDE=	-iinclude: 

CFLAGS=		-cfimsq -rr -m2 -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS=		-o$(OBJ) -iinclude: 
LFLAGS=		BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES=	lib:lcr.lib lib:small.lib lib:debug.lib

LIBNAME=	playerprefs.library

all:	_all
	execute <<
failat 1;
cd Player;
lmk $*;
cd /BasicIO;
lmk $*;
cd /ScreenSaver;
lmk $*;
cd /TradeMark;
lmk $*;
cd /;
lmk pplibrary.ld;
<

_all:
	@echo "Building playerprefs.library..."


pplibrary.ld:   $(OFILES) $(LASTO)
                blink <with <(temp2_lmk.tmp)
FROM $(OFILES) +
$(PLAYEROFILES) +
$(SSOFILES) +
$(TMOFILES) +
$(BASICOFILES) +
$(LASTO)
TO playerprefs.library
LIBPREFIX _LIB LIBFD playerprefs_private_lib.fd
BATCH SMALLCODE NOICONS
VER ERRORFILE LIB $(LIBRARIES) MAP playerprefs.map F H L O S X PLAIN
<
                copy playerprefs.library playerprefs.ld.cr
                blink playerprefs.ld.cr to df0:modules/playerprefs.ld.strip nd

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


$(OBJ)commoncode.o:     commoncode.c playerprefsbase.h playerprefs.h

$(OBJ)display.o:        display.c playerprefsbase.h playerprefs.h

$(OBJ)libent.o:         libent.a playerprefs_rev.i playerprefsbase.i playerprefs_rev.i
                        asm -u -iCDTV:include/ -iV37:include/ -iinclude: -o$(OBJ)libent.o libent.a

$(OBJ)libfunc.o:        libfunc.a

$(OBJ)animcontrol.o:    animcontrol.a animate.i
			hx68 -a $*.a -csy -i include: -o $(OBJ)$*.o

$(OBJ)enddata.o:        enddata.a

internal:	_internal
	execute <<
cd Player;
lmk $*;
cd /BasicIO;
lmk $*;
cd /ScreenSaver;
lmk $*;
cd /TradeMark;
lmk $*;
cd /;
<

_internal:
	copy $(CFILES) $(SRCDIR) nopro
	copy $(HFILES1) $(SRCDIR) nopro
	copy $(HFILES2) $(SRCDIR) nopro
	copy $(HFILES3) $(SRCDIR) nopro
	copy $(AFILES) $(SRCDIR) nopro
	copy $(IFILES) $(SRCDIR) nopro
	copy $(MISCFILES1) $(SRCDIR) nopro
	copy $(MISCFILES2) $(SRCDIR) nopro
# Temporarily, we copy the .ld file to softserve to circumvent an
# AS225/NFS bug which sometimes makes the file unreadable from heartofgold.
	copy playerprefs.ld.cr softserve:peter/release

clean:	_clean

	execute <<
cd Player;
lmk $*;
cd /BasicIO;
lmk $*;
cd /ScreenSaver;
lmk $*;
cd /TradeMark;
lmk $*;
cd /;
<

_clean:
	delete obj/\#?.o playerprefs.ld.cr playerprefs.library
