#  To keep objects in a separate directory (useful for compiling with
#  floppy based system), set OD (object directory) to the desired
#  directory.

ROOT =		/
OD =		/amiga
SD =		/common
MD =		/amiga

#DBUG =		-dDBUG=1
DBUG =		-dDBUG=0

#TESTONLY =	-dTESTONLY=1
TESTONLY =	-dTESTONLY=0

#  Flags specific to Lattice C compiler:
#
#	-cf	Force all functions to have prototypes in scope and
#		complain if no prototype.
#	-j304i	Turn off spurious "dead assignment eliminated" warnings.
#	-H<f>	Load symbol table from precompiled header file <f>.
#	-D	Use PRECOMPILED_HEADERS to avoid reincluding globals.h

LATTICEFLAGS =	-cf -j304i -Hgbls.sym -DPRECOMPILED_HEADERS=1

OFLAG =		-O
DEFS =
COPYRIGHT =
INCLUDE =	-i$(SD)/
PROF =
OTHERCFLAGS =	$(LATTICEFLAGS)
OTHERLDFLAGS =
LIBC =
LIBDBUG =
LIBS =		$(LIBDBUG) $(LIBC)
CC =		lc
LD =		blink

CDEFINES =	$(DEFS) $(DBUG) $(TESTONLY) $(COPYRIGHT)
CFLAGS =	$(OFLAG) $(CDEFINES) $(INCLUDE) $(PROF) $(OTHERCFLAGS)
HCFLAGS =	$(OFLAG) $(CDEFINES) $(INCLUDE) -ph
LDFLAGS =	$(OTHERLDFLAGS)
LINTLIBS =	$(LIBS) $(LIBDBUG)
LINTDEFS =	$(DEFS) $(INCLUDE) $(TESTONLY)

S1 =		$(SD)/access.c $(SD)/blocks.c $(SD)/bru.c $(SD)/brutab.c 
S2 =		$(SD)/chksum.c $(SD)/create.c $(SD)/date.c $(SD)/devices.c
S3 =		$(SD)/diff.c $(SD)/done.c $(SD)/estimate.c $(SD)/usermsg.c
S4 =		$(SD)/execute.c $(SD)/extract.c $(SD)/fmode.c $(SD)/globals.c
S5 =		$(SD)/group.c $(SD)/headers.c $(SD)/hex.c $(SD)/info.c
S6 =		$(SD)/init.c $(SD)/inspect.c $(SD)/links.c $(SD)/passwd.c
S7 =		$(SD)/readinfo.c $(SD)/scan.c $(SD)/signals.c $(SD)/symlinks.c
S8 =		$(SD)/sys.c $(SD)/sys2.c $(SD)/sys3.c $(SD)/sys4.c $(SD)/table.c
S9 =		$(SD)/trees.c $(SD)/tty.c $(SD)/usage.c $(SD)/utils.c $(SD)/wild.c
SA =		$(MD)/filestat.c $(MD)/libaccess.c $(MD)/rawdev.c
SB =		$(MD)/mkdir.c $(MD)/utime.c $(MD)/timeconv.c $(MD)/attributes.c
SC =		$(MD)/expandargs.c $(MD)/alloflop.c $(MD)/signal.c
SE =		$(SD)/compress.c $(MD)/mktemp.c $(SD)/filter.c $(MD)/ipc.c
SF =		$(MD)/rawtape.c $(MD)/stackcheck.c
CFILES =	$(S1) $(S2) $(S3) $(S4) $(S5) $(S6) $(S7) $(S8) $(S9) \
		$(SA) $(SB) $(SC) $(SE) $(SF)

O1 =		access.o blocks.o bru.o brutab.o 
O2 =		chksum.o create.o date.o devices.o
O3 =		diff.o done.o usermsg.o estimate.o
O4 =		execute.o extract.o fmode.o globals.o
O5 =		group.o headers.o hex.o info.o
O6 =		init.o inspect.o links.o passwd.o
O7 =		readinfo.o scan.o signals.o symlinks.o
O8 =		sys.o sys2.o sys3.o sys4.o table.o
O9 =		trees.o tty.o usage.o utils.o wild.o
OA =		filestat.o libaccess.o rawdev.o
OB =		mkdir.o utime.o timeconv.o attributes.o
OC =		expandargs.o alloflop.o signal.o
OE =		compress.o mktemp.o filter.o ipc.o
OF =		rawtape.o stackcheck.o
OFILES =	$(O1) $(O2) $(O3) $(O4) $(O5) $(O6) $(O7) $(O8) $(O9) \
		$(OA) $(OB) $(OC) $(OE) $(OF)

HFL1 =		$(SD)/dbug.h $(SD)/manifest.h $(SD)/typedefs.h $(SD)/config.h
HFL2 =		$(SD)/usermsg.h $(SD)/blocks.h $(SD)/macros.h $(SD)/finfo.h 
HFL3 =		$(SD)/devices.h $(SD)/flags.h $(SD)/bruinfo.h $(SD)/exeinfo.h
HFL4 =		$(SD)/trees.h $(SD)/dir.h $(SD)/rmt.h $(SD)/utsname.h
HFILES =	$(HFL1) $(HFL2) $(HFL3) $(HFL4)

bru :			$(OFILES)
			$(LD) with link.cmd

gbls.sym :		$(SD)/globals.h
			copy $(SD)/globals.h gbls.c
			$(CC) $(HCFLAGS) -o$@ gbls.c
			delete gbls.c

mktemp.o :		$(MD)/mktemp.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/mktemp.c

attributes.o :		$(MD)/attributes.c gbls.sym
			$(CC) $(CFLAGS) -o$@ attributes.c

tty.o :	 		$(SD)/tty.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/tty.c

execute.o :		$(SD)/execute.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/execute.c

utils.o :		$(SD)/utils.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/utils.c

access.o :		$(SD)/access.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/access.c

sys4.o :		$(SD)/sys4.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/sys4.c

hex.o :			$(SD)/hex.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/hex.c

wild.o :		$(SD)/wild.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/wild.c

compress.o :		$(SD)/compress.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/compress.c

create.o :		$(SD)/create.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/create.c

usage.o :		$(SD)/usage.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/usage.c

group.o :		$(SD)/group.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/group.c

estimate.o :		$(SD)/estimate.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/estimate.c

chksum.o :		$(SD)/chksum.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/chksum.c

devices.o :		$(SD)/devices.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/devices.c

done.o :		$(SD)/done.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/done.c

rawdev.o :		$(MD)/rawdev.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/rawdev.c

rawtape.o :		$(MD)/rawtape.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/rawtape.c

diff.o :		$(SD)/diff.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/diff.c

inspect.o :		$(SD)/inspect.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/inspect.c

ipc.o :			$(MD)/ipc.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/ipc.c

sys2.o :		$(SD)/sys2.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/sys2.c

usermsg.o :		$(SD)/usermsg.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/usermsg.c

readinfo.o :		$(SD)/readinfo.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/readinfo.c

extract.o :		$(SD)/extract.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/extract.c

headers.o :		$(SD)/headers.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/headers.c

blocks.o :		$(SD)/blocks.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/blocks.c

table.o :		$(SD)/table.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/table.c

filter.o:		$(SD)/filter.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/filter.c

fmode.o :		$(SD)/fmode.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/fmode.c

signals.o :		$(SD)/signals.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/signals.c

symlinks.o :		$(SD)/symlinks.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/symlinks.c

links.o :		$(SD)/links.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/links.c

globals.o :		$(SD)/globals.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/globals.c

mkdir.o :		$(MD)/mkdir.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/mkdir.c

utime.o :		$(MD)/utime.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/utime.c

timeconv.o :		$(MD)/timeconv.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/timeconv.c

expandargs.o :		$(MD)/expandargs.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/expandargs.c

alloflop.o :		$(MD)/alloflop.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/alloflop.c

signal.o :		$(MD)/signal.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/signal.c

sys3.o :		$(SD)/sys3.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/sys3.c

brutab.o :		$(SD)/brutab.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/brutab.c

bru.o :			$(SD)/bru.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/bru.c

sys.o :			$(SD)/sys.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/sys.c

passwd.o :		$(SD)/passwd.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/passwd.c

init.o :		$(SD)/init.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/init.c

trees.o :		$(SD)/trees.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/trees.c

stackcheck.o :		$(MD)/stackcheck.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/stackcheck.c

filestat.o :		$(MD)/filestat.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/filestat.c

libaccess.o :		$(MD)/libaccess.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(MD)/libaccess.c

scan.o :		$(SD)/scan.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/scan.c

date.o :		$(SD)/date.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/date.c

info.o :		$(SD)/info.c gbls.sym
			$(CC) $(CFLAGS) -o$@ $(SD)/info.c
