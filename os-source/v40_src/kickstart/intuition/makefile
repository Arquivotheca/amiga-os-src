

#  $Id: Makefile,v 38.15 93/02/15 18:55:59 peter Exp $

VERSION=	40

SUBSYSNAME=intuition
SUBSYSGROUP=intuition
SRCDIR=kickstart/intuition
V=V$(VERSION)
VI=/usr/commodore/amiga/$(V)/include
MYDEBUGLIB=idebug.lib

# log file for RCS
RCSLOG=Log

all:		${MYDEBUGLIB} intuition.ld echovers

echovers:
	@echo   '--> intuition.library' $(VERSION)'.'`cat intuition_rev.rev`

pdown:	all
# Download to peter's A3000
	@echo	"Updating soda's libs:intuition.library..."
	@rcp	intuition.ld soda:libs:intuition.library
	@echo	"Updating soda's f00000 intuition..."
	@rsh	soda loadf libs:intuition.library
	@rsh	soda enforcer >NIL: off
	@echo	'Rebooting soda...'
	@rsh	soda run s:reboot_delay


pdown2:	all
# Download to peter's A2000
	@echo	do remote download on ginger to bb0
	rsh	ginger rram  ~peter/intuition/intuition.ld

thebigone:	all intuition.doc internal iflow tags

DEBUG=		${LIBDIR}/debug.lib
MYLIBS=		${MYDEBUGLIB} ${DEBUG} ${LIBDIR}/rom.lib endcode.lib
#MYLIBS=		${MYDEBUGLIB} ${DEBUG}
#MYLIBS=	${LIBDIR}/rom.lib

#AFLAGS=		-c S
#CAFLAGS=		-l $*.lis
#CAPP=		sed -f cappfile 
CAPP=		capp.sh

CFLAGS=		-X105 -X107
# These are used in iflow:
CPPFLAGS=	 -DIFLOW=1
# LFLAGS=		map=$*.lmap xref=$*.xref
#LFLAGS=		ROM map=$*.lmap
#LFLAGS=		map $*.lmap ROM
LFLAGS=		ROM

EXTERNHFILES=	intuition.h intuitionbase.h screens.h preferences.h \
		cghooks.h sghooks.h classes.h classusr.h \
		imageclass.h gadgetclass.h icclass.h pointerclass.h \
		iobsolete.h

INTERNHFILES=		iprefs.h

MYHFILES=		intuall.h\
			intuinternal.h \
			ifunctions.h	\
			i.h \
			ism.h \
			sc.h \
			ibase.h \
			ezreq.h \
			vectorclass.h \
			newlook.h

HFILES=			$(MYHFILES) \
			$(INTERNHFILES) \
			$(EXTERNHFILES) 


EXTERNIFILES=	intuition.i intuitionbase.i screens.i preferences.i \
		cghooks.i sghooks.i classes.i classusr.i \
		imageclass.i gadgetclass.i icclass.i pointerclass.i \
		iobsolete.i

IFILES=			$(EXTERNIFILES) \
			intuall.i	\
			ibase.i		\
			vectorclass.i

CLASSCFILES= 	pubclasses.c classes.c rootclass.c gadclass.c propgadclass.c \
		imageclass.c ggclass.c icclass.c modclass.c \
		buttongclass.c strgadclass.c \
		frbuttonclass.c itexticlass.c fillrectclass.c frameiclass.c


CLASSAFILES=	classface.asm

AFILES=		iwork.asm intuitionface.asm \
		downcode.asm intuileap.asm lockstub.asm \
		mousebuttons.asm abs.asm hookface.asm aformat.asm \
		sysreqstack.asm xsysidata.asm $(CLASSAFILES)

# boxiclass.c twostateclass.c

VECTORCFILES=	vectorclass.c  xsysiclass.c
ISMCFILES=	ism.c iequeue.c ih.c ihfront.c isdefault.c pools.c \
		sidlewindow.c snowindow.c sgadget.c swsizedrag.c \
		sscreendrag.c smenu.c srequester.c sverify.c \
		sdmrtimeout.c

CFILES=		$(CLASSCFILES) $(ISMCFILES) \
		$(VECTORCFILES)  \
		activate.c alert.c bhooks.c coercion.c \
		dclip.c \
		drawgadgets.c ezreq.c fonts.c format.c \
		gadgets.c \
		genv.c hitgadgets.c ihooks.c \
		idcmp.c ilock.c images.c  init.c \
		iprefs.c isupport.c isupp2.c\
		layout.c \
		menu.c misc.c mouse.c \
		newlook.c \
		phooks.c pointer.c prop.c prefs.c pubscreen.c \
		rect.c requester.c \
		scale.c screens.c scsupp.c shooks.c sinput.c size.c \
		strings.c \
		thing.c timing.c varargs.c view.c \
		wbench.c windows.c wtags.c zoom.c autodocs.c

FIRSTO=		abs.obj

VECTOROFILES=	vectorclass.obj  xsysiclass.obj xsysidata.obj

ISMOFILES=	ism.obj iequeue.obj ih.obj ihfront.obj isdefault.obj pools.obj \
		sidlewindow.obj snowindow.obj sgadget.obj swsizedrag.obj \
		sscreendrag.obj smenu.obj srequester.obj sverify.obj \
		sdmrtimeout.obj

CLASSOFILES= 	pubclasses.obj classes.obj rootclass.obj \
		gadclass.obj propgadclass.obj \
		imageclass.obj ggclass.obj icclass.obj modclass.obj \
		buttongclass.obj strgadclass.obj \
		frbuttonclass.obj frameiclass.obj  \
		itexticlass.obj fillrectclass.obj classface.obj

newlook.obj windows.obj: newlook.h

# using vector images
#		$(IMAGEOFILES)

# We're now using short-vectors in the library table, so a number of
# modules need to be proximate to iwork.obj.  Most entries are in
# intuitionface.obj, but there are some in screens.obj, misc.obj,
# downcode.obj, view.obj, wbench.obj, pubscreen.obj, isupport.obj,
# ih.obj (ISMOFILES), and pubclasses.obj (CLASSOFILES).

OFILES=		iwork.obj	\
		intuitionface.obj \
		downcode.obj \
		screens.obj \
		misc.obj \
		view.obj \
		wbench.obj \
		pubscreen.obj \
		isupport.obj \
		$(CLASSOFILES) $(ISMOFILES) \
		$(VECTOROFILES) \
		activate.obj alert.obj bhooks.obj coercion.obj \
		dclip.obj \
		drawgadgets.obj ezreq.obj fonts.obj format.obj gadgets.obj \
		genv.obj hitgadgets.obj hookface.obj ihooks.obj \
		idcmp.obj ilock.obj images.obj init.obj \
		iprefs.obj isupp2.obj\
		layout.obj lockstub.obj menu.obj mouse.obj \
		newlook.obj \
		phooks.obj pointer.obj prop.obj prefs.obj \
		rect.obj requester.obj \
		scale.obj scsupp.obj shooks.obj \
		sinput.obj size.obj  strings.obj \
		thing.obj timing.obj varargs.obj \
		windows.obj wtags.obj zoom.obj\
		intuileap.obj mousebuttons.obj \
		aformat.obj sysreqstack.obj autodocs.obj

MYLIBOFILES=	dump.obj kstuff.obj dbrefresh.obj
MYLIBSRC=	dump.c kstuff.c dbrefresh.c \
		endcode.asm

MISCSRC=	$(MYLIBSRC) cappfile capp.sh offsets.c


# using makemeta definition of RCSFILES
# files under RCS control (note, no intuition_rev.rev at this time)
#RCSFILES=	$(AFILES) \
#		$(CFILES) \
#		$(MAKEFILE) \
#		$(HFILES) \
#		$(MISCSRC)  \
#		$(MYLIBSRC)  \
#		$(IFILES)	\
#		$(SUBSYSNAME)_lib.fd

# to get version bump
abs.obj:	intuition_rev.i

intuition_rev.i: intuition_rev.rev $(OFILES)
	@rm -f intuition_rev.i
	@${BUMPREV} ${VERNUM} intuition_rev

DEPENDS= abs.asm intuition_rev.i

MAKEFILE=	Makefile

.INCLUDE=/usr/commodore/amiga/$(V)/tools/makemeta
#.INCLUDE=makemeta.local

${MYDEBUGLIB}: ${MYLIBOFILES}
	cat ${MYLIBOFILES} > ${MYDEBUGLIB}

endcode.lib: endcode.obj
	cp endcode.obj endcode.lib

$(RCSLOG): $(RCSFILES)
	changelog  -u -f $(RCSLOG) $(RCSFILES)

# get a file with the RCSFILES listed, for all these
# bloody global RCS changes I keep needing to make.
rcslist: $(MAKEFILE)
	echo $(RCSFILES) >rcslist.raw
	 tr -s '\040' '\012' <rcslist.raw >rcslist

# used to change $Id: Makefile,v 38.15 93/02/15 18:55:59 peter Exp $
# modified for other global source file changes.

IDTAG=Id
rcsupdate:
#	co -l ${RCSFILES}
	for i in ${RCSFILES} ; do ( \
	sed -e 's/$$Id: Makefile,v 38.15 93/02/15 18:55:59 peter Exp $
	mv krell.tmp $$i \
	); done
	ci -f -u -m"use Id instead of Header for 4.x rcs" ${RCSFILES}

rcstest:
	for i in ${RCSFILES} ; do ( \
	echo 's/$$Id: Makefile,v 38.15 93/02/15 18:55:59 peter Exp $
	); done


# automatically generate offset constants 
iwork.obj: ioffsets.i

ioffsets.i: intuinternal.h ibase.h  offsets.c
	cc $(CINCL) offsets.c -o offsets
	offsets > ioffsets.i

includelist:
	@echo $(EXTERNHFILES) $(EXTERNIFILES)  >includelist

iflow: $(CFILES)
	calls -w256 $(CINCL) $(CPPFLAGS) $(CFILES) | expand > iflow

jtags:	$(CFILES)
	echo myhfiles: $(MYHFILES)
	xtags $(CFILES) $(MYLIBSRC) $(HFILES)
