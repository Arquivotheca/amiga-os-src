######################################################################
MAKEMETA=	../../tools/makemeta

SUBSYSNAME=	cstrings

AFILES=		TailPath.asm bcopy.asm bzero.asm index.asm \
		rindex.asm scopy.asm sfree.asm strcat.asm \
		strcmp.asm strcpy.asm strlen.asm strncat.asm \
		strncmp.asm strncpy.asm suffix.asm

OFILES=		TailPath.obj bcopy.obj bzero.obj index.obj \
		rindex.obj scopy.obj sfree.obj strcat.obj \
		strcmp.obj strcpy.obj strlen.obj strncat.obj \
		strncmp.obj strncpy.obj suffix.obj

all quick:	${SUBSYSNAME}.lib
		cp ${SUBSYSNAME}.lib ..

.INCLUDE=${MAKEMETA}
