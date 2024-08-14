#
# the name of the subsystem to make
#
MAKEFILE=	Makefile
SUBSYSNAME=	hddisk
SRCDIR=		hddisk

# the include and link lines to use
#
#AFLAGS=		-l $*.lis
#LFLAGS=		map=$*.lmap xref=$*.xref

EXTERNIFILES=	hddisk.i
IFILES=		internal.i messages.i asmsupp.i ${EXTERNIFILES} \
		    ${SUBSYSNAME}_rev.i

EXTERNHFILES=	
HFILES=		${EXTERNHFILES} ${SUBSYSNAME}_rev.h

SUBSYSGROUP=	devices

LFILES=		${LIBDIR}/amiga.lib
MYLIBS=		$(LIBDIR)/debug.lib

#
# OLFILES are the list of files that are linked in
#
FIRSTO= abs.obj

# endcode MUST be last

AFILES= abs.asm badtrans.asm buffers.asm driver.asm format.asm hdintrpt.asm \
	init.asm io.asm scsi.asm task.asm endcode.asm

OFILES= badtrans.obj buffers.obj driver.obj format.obj hdintrpt.obj \
	init.obj io.obj scsi.obj task.obj endcode.obj
#
# The main dependency line: everything else follows from this
#
all: ${SUBSYSNAME}.ld

strip:
	astrip hddisk.ld

.INCLUDE=/usr/commodore/amiga/V/tools/makemeta
