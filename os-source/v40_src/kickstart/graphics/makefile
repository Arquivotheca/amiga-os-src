##########################################################################
#
# $Id: lmkfile,v 1.6 91/08/13 10:38:17 chrisg Exp $
#
# $Locker:  $
#
##########################################################################

VERSION=	38
SRCDIRPATH=     gfxsrc:
V=              V:
SUBDIRS=	a c d gels text fonts
SRCDIR=		graphics
SUBSYSNAME=	graphics
SUBSYSGROUP=	graphics
DESTDIR=	v:src/kickstart/graphics
DESTINCDIR=	v:include

.asm.obj:
	casm -A $*.asm -iincluder: -o$*.obj

.SUFFIXES: .c .obj

GFXLIBS= a/a.lib c/c.lib d/d.lib gels/gels.lib text/text.lib fonts/fonts.lib

MYLIBS=	${GFXLIBS} ${V}lib/rom.lib ${V}lib/debug.lib

AFILES=	pwrup.asm

OFILES=	pwrup.obj

EXTERNIFILES1= clip.i copper.i display.i gels.i gfx.i gfxbase.i gfxnodes.i
EXTERNIFILES2= layers.i monitor.i rastport.i regions.i sprite.i view.i 
EXTERNIFILES3= displayinfo.i videocontrol.i text/scale.i text/text.i

IFILES= \
	cia8520.i \
	sane_names.i \
	macros.i \
	displayinfo_internal.i \
	${EXTERNIFILES1} \
	${EXTERNIFILES2} \
	${EXTERNIFILES3}

EXTERNHFILES1= clip.h collide.h copper.h display.h displayinfo.h gels.h gfx.h 
EXTERNHFILES2= gfxbase.h gfxmacros.h gfxnodes.h graphint.h layers.h monitor.h
EXTERNHFILES3= rastport.h regions.h sprite.h view.h videocontrol.h text/scale.h text/text.h

HFILES= \
	cia8520.h \
	sane_names.h \
	macros.h \
	displayinfo_internal.h \
	${EXTERNHFILES1} \
	${EXTERNHFILES2} \
	${EXTERNHFILES3}

FIRSTO=	pwrup.obj

MFLAGS=	VERSION=${VERSION}

LFLAGS=	SMALLCODE SMALLDATA ADDSYM XREF graphics.xref MAP graphics.lmap XOHSFL PLAIN

all:	pwrup.obj

	@echo >ram:x "cd a"
	@echo >>ram:x "lmk"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd c"
	@echo >>ram:x "lmk"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd d"
	@echo >>ram:x "lmk"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd gels"
	@echo >>ram:x "lmk"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd text"
	@echo >>ram:x "lmk"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd fonts"
	@echo >>ram:x "lmk"
	@echo >>ram:x "cd /"
	@execute ram:x
	blink <WITH <
	FROM $(FIRSTO) LIB $(MYLIBS) to graphics.ld
	$(LFLAGS)
	<
	@run lmk saveobjs >con:320/10/640/40/Savaobjs

pwrup.obj:	pwrup.asm graphics_rev.i gfxbase.i


# bump the revision number

rev:
	@rx >* bumprev.rexx $(SRCDIRPATH) $(VERSION)


# delete all the .obj files

clean:
	@echo "Say goodbye to those .obj files....."
	-delete *.obj *ld*
	@echo >ram:x "cd a"
	@echo >>ram:x "lmk clean"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd c"
	@echo >>ram:x "lmk clean"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd d"
	@echo >>ram:x "lmk clean"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd fonts"
	@echo >>ram:x "lmk clean"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd gels"
	@echo >>ram:x "lmk clean"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd text"
	@echo >>ram:x "lmk clean"
	@echo >>ram:x "cd /"
	@execute ram:x

# protos..

protos:
	@echo "Making lattice protos"
	@echo >ram:x "cd c"
	@echo >>ram:x "lmk protos"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd d"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd gels"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@execute ram:x

# get back my .obj files!....

saveobjs:
	@echo "Save those .obj files onto disk..."
	@echo >ram:x "cd a"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd c"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd d"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd fonts"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd gels"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@echo >>ram:x "cd text"
	@echo >>ram:x "lmk saveobjs"
	@echo >>ram:x "cd /"
	@execute ram:x

# the autodocs....

doc:
	@echo "Making autodocs. This will write to v:doc!"
	join a/*.asm c/*.(c|asm) d/*.(c|asm) fonts/*.(c|asm) gels/*.(c|asm) text/*.(c|asm) to ram:x
	autodoc ram:x >graphics.doc
	delete ram:x
	copy graphics.doc v:doc

# to the server...

internal:
	@echo "Make internal - Server, or bust!"
	stripf graphics.ld graphics.ld.strip
	copy ** $(DESTDIR) nopro
	copy a/*.(c|asm|h|i) $(DESTDIR)/a nopro
	copy c/*.(c|asm|h|i) $(DESTDIR)/c nopro
	copy d/*.(c|asm|h|i) $(DESTDIR)/d nopro
	copy fonts/*.(c|asm|h|i) $(DESTDIR)/fonts nopro
	copy gels/*.(c|asm|h|i) $(DESTDIR)/gels nopro
	copy text/*.(c|asm|h|i) $(DESTDIR)/text nopro
	copy graphics_lib.sfd v:lib/sfd nopro
	
# include files....

headers:
	@echo "Copying all include files over to their homes..."
	@copy $(EXTERNHFILES1) $(DESTINCDIR)/$(SUBSYSGROUP)
	@copy $(EXTERNHFILES2) $(DESTINCDIR)/$(SUBSYSGROUP)
	@copy $(EXTERNHFILES3) $(DESTINCDIR)/$(SUBSYSGROUP)
	@copy $(EXTERNIFILES1) $(DESTINCDIR)/$(SUBSYSGROUP)
	@copy $(EXTERNIFILES2) $(DESTINCDIR)/$(SUBSYSGROUP)
	@copy $(EXTERNIFILES3) $(DESTINCDIR)/$(SUBSYSGROUP)

#stripper? I hardly even know her.
#	@rx stripper $(SRCDIRPATH),$(EXTERNHFILES1),$(DESTINCDIR).strip/$(SUBSYSGROUP)
#	@rx stripper $(SRCDIRPATH),$(EXTERNHFILES2),$(DESTINCDIR).strip/$(SUBSYSGROUP)
#	@rx stripper $(SRCDIRPATH),$(EXTERNHFILES3),$(DESTINCDIR).strip/$(SUBSYSGROUP)
#	@rx stripper $(SRCDIRPATH),$(EXTERNIFILES1),$(DESTINCDIR).strip/$(SUBSYSGROUP)
#	@rx stripper $(SRCDIRPATH),$(EXTERNIFILES2),$(DESTINCDIR).strip/$(SUBSYSGROUP)
#	@rx stripper $(SRCDIRPATH),$(EXTERNIFILES3),$(DESTINCDIR).strip/$(SUBSYSGROUP)
