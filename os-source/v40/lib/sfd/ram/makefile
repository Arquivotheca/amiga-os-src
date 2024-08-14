######################################################################
MAKEMETA=	../../../tools/makemeta

# The list of modules is maintained by hand.  If you add a module,
# you must add it to V:lib/sfd/Makefile, V:lib/sfd/(RAM|ROM|LVO)/Makefile,
# and mkdir V:lib/sfd/(RAM|ROM)/subsystem!

SUBDIRS=	amigaguide \
		asl \
		battclock \
		battmem \
		bullet \
		cardres \
		cia \
		colorwheel \
		commodities \
		console \
		datatypes \
		disk \
		diskfont \
		dos \
		exec \
		expansion \
		gadtools \
		graphics \
		icon \
		iffparse \
		input \
		intuition \
		keymap \
		layers \
		locale \
		lowlevel \
		mathffp \
		mathieeedoubbas \
		mathieeedoubtrans \
		mathieeesingbas \
		mathieeesingtrans \
		mathtrans \
		misc \
		nonvolatile \
		potgo \
		ramdrive \
		realtime \
		rexxsyslib \
		timer \
		translator \
		utility \
		wb

all:		${SUBDIRS} amiga.lib

quick:		amiga.lib

amiga.lib:	FRC
		rm -f amiga.lib
		for i in ${SUBDIRS} ; do cat $${i}.lib >>amiga.lib; done

.INCLUDE=${MAKEMETA}
