######################################################################
#
#	$Log:	Makefile,v $
# Revision 36.4  91/11/05  19:18:12  peter
# Subsystem name is now ddebug_lib
# 
# Revision 36.3  91/11/05  13:42:12  peter
# Doc file is now moved into V:lib.
# 
# Revision 36.2  91/11/05  13:21:58  peter
# Added 'doc' to the "all" rule.
# 
# Revision 36.1  90/07/26  00:35:56  bryce
# Merge into submake (so everything is automatic)
# 
#
MAKEMETA=	../../tools/makemeta

SUBSYSNAME=	ddebug_lib

RESULTFILE=	ddebug.lib

AFILES= 	tty.asm cmpstr.asm getnum.asm rawio.asm
IFILES=		assembly.i

OFILES= 	tty.obj cmpstr.obj getnum.obj rawio.obj

all quick:	${SUBSYSNAME}.lib doc
		cp ${SUBSYSNAME}.lib ../${RESULTFILE}
		mv ${SUBSYSNAME}.doc ..

.INCLUDE=${MAKEMETA}
