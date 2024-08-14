# make file for installer under SAS/C

CFLAGS = -cusmwf -d0 -v
# CFLAGS = -cusmwf -d0 -v -dONLY2_0=1
AFLAGS = 

AINC = dh2:asm2.0/

WINOBJ	= window.o layout.o colors.o files.o support.o derr.o \
 action.o inst.o instverlat.o text.o to_english.o strings.o
INSOBJ  = compile.o interp.o statement.o symtab.o lparse.o \
 start.o match.o scan.o
XTRAOBJ = customgad.o
V1OBJ = pmatch.o patch.o
XLIB = xliba.o xlibc.o

#XLIB = printf.o path.o tackon.o makebitmap.o\
# memalloc.o sprintf.o getfilesize.o clamp.o zero.o gethead.o\
# nextnode.o disktype.o cpycstrbstr.o initsimplepacket.o\
# sendpacket.o getfiledate.o enviroment.o\
# transferlist.o gets.o fileonly.o puts.o gettail.o\
# initpacket.o cpybstrcstr.o a_to_x.o

.c.o:
	lc $(CFLAGS) -o$*.o $*.c

.asm.o:
	asm $(AFLAGS) -i$(AINC) -o$*.o $*.asm

final: $(WINOBJ) $(INSOBJ) $(XTRAOBJ) $(XLIB)
	blink WITH installer.with

old: $(WINOBJ) $(INSOBJ) $(V1OBJ) $(XLIB)
	blink WITH instold.with

test.o: text.h

strings.o: cat_text.asm

instverlat.o: Installer_rev.h Installer_2_rev.h

swap:
	setdate layout.c
	setdate window.c
	setdate text.c
	setdate action.c
	setdate colors.c
	setdate files.c
	setdate compile.c
	setdate instverlat.c

bump:
	bumprev 1 installer_rev

bump2:
	bumprev 2 installer_2_rev
