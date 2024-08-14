SRCDIR=	CDGS:src/pplibrary
SUBDIR=	Player

OBJ = obj/

INCLUDE =       -iinclude: 

CFLAGS =        -cfimsq -rr -m2 -ml -ms -. -O -v -d1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 

CFILES=		cdg.c player.c gadgets.c
MISCFILES=	makefile scrdata.pic scrbuttons.pic cdgnumb.pic

ASSEMBLER         = HX68 -a
ASSEMBLER_OPTS    = -csy -i include: -o

_all:	player

player:         $(OBJ)cdg.o $(OBJ)gadgets.o $(OBJ)player.o \
                $(OBJ)scrdata.o $(OBJ)scrbuttons.o $(OBJ)cdgnumb.o

.c.o:
        assign ppl: /
        lc $(CFLAGS) $*.c

.a.o:
	$(ASSEMBLER) $*.a $(ASSEMBLER_OPTS) $(OBJ)$*.o

$(OBJ)player.o:         player.c /playerprefs.h /playerprefsbase.h /basicio/keyclick.h

$(OBJ)cdg.o:            cdg.c /playerprefs.h /playerprefsbase.h

$(OBJ)gadgets.o:        gadgets.c /playerprefs.h

$(OBJ)scrdata.o:        scrdata.pic
	# sbox with ranges but no colors
	# We crash if we decompress pics made with the new sbox, but the old
	# sbox doesn't give us 8-bit color, so we enclose the color table
	# in source code elsewhere.
        sbox -x -c $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x

$(OBJ)scrbuttons.o:     scrbuttons.pic
	# sbox with no colors or ranges, but with mask
	sbox -x -c -r -M $*.pic ram:$*.x
	d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
	delete ram:$*.x

$(OBJ)cdgnumb.o:        cdgnumb.pic
	# sbox with colors but no ranges
        sbox -x -r $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x

#invoked by make internal in the parent directory
_internal:
	copy $(CFILES) $(SRCDIR)/$(SUBDIR) nopro
	copy $(MISCFILES) $(SRCDIR)/$(SUBDIR) nopro


_clean:
	delete obj/\#?.o
