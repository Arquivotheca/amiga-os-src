##########################################################################
#
#	Thu Jul 22 00:10:55 EDT 1990	Added small.lib -Bryce
#	Thu Jul 26 16:13:11 EDT 1990	Added ddebug -Bryce
#
MAKEMETA=	../tools/makemeta

SUBDIRS=	csupport sfd hardware cstrings debug ddebug \
		startup version

LIBFILES=	amiga.lib rom.lib small.lib cstrings.lib ddebug.lib debug.lib

AMIGALIBS=	version/amiga.lib sfd/amiga.lib csupport/amiga.lib \
		hardware/hardware_lib.lib

ROMLIBS=	version/rom.lib sfd/rom.lib csupport/rom.lib \
		hardware/hardware_lib.lib

SMALLLIBS=	version/small.lib sfd/small.lib csupport/amiga.lib \
		hardware/hardware_lib.lib

DOCFILES=	amiga_lib.doc debug_lib.doc ddebug_lib.doc

all quick:	${SUBDIRS}
		cat ${AMIGALIBS} >amiga.lib.TMP
		mv amiga.lib.TMP amiga.lib
		cat ${ROMLIBS} >rom.lib.TMP
		mv rom.lib.TMP rom.lib
		cat ${SMALLLIBS} >small.lib.TMP
		mv small.lib.TMP small.lib
		${MMINSTALL} ${DOCDIR} ${DOCFILES}
		chmod 660 ${LIBFILES}

clean::
		-touch autofd/*.fd

.INCLUDE=${MAKEMETA}
