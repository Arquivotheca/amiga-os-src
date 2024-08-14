
VERSION=	34
V=      /usr/commodore/amiga/V$(VERSION)

SRCDIR=		bootrom

# the following have defaults in the system makefile
# use these lines to add user options
#
AINCL=
CINCL=
AFLAGS=
CFLAGS=
LFILES=

MAKEFILE=Makefile

DEBUG=	$V/internal/lib/debug.lib

MYLIBS=	$(DEBUG)
 
SUBSYSNAME=	bootrom

AFILES= bootrom.asm \
hdintrpt.asm\
buffers.asm\
init.asm\
badtrans.asm\
driver.asm\
format.asm\
io.asm\
scsi.asm\
task.asm

CFILES=

EXTERNIFILES=
IFILES=	${EXTERNIFILES} \
	bootrom.i

EXTERNHFILES=
HFILES=	${EXTERNHFILES}

STARTUP=

#
FIRSTO= bootrom.obj
OFILES= \
hdintrpt.obj\
buffers.obj\
init.obj\
badtrans.obj\
driver.obj\
format.obj\
io.obj\
scsi.obj\
task.obj

all:	${SUBSYSNAME}.ld 

${SUBSYSNAME}.ld: ${OFILES}

bootrom.obj:	bootrom.asm bootrom.i

.INCLUDE=$V/tools/makemeta
