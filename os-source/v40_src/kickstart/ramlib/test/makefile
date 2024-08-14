V=		/usr/commodore/amiga/V36
STARTUP=	$V/lib/startup.obj
MYLIBS=		$V/lib/debug.lib
TEXTOBJS=	$(STARTUP) test.obj $(MYLIBS)

all:		test.ld

test.ld:	$(TEXTOBJS)
		echo >$*.with "from $(TEXTOBJS)"
		${LINK} with=$*.with library=${LIBDIR}/amiga.lib \
			to=$*.ld ${LFLAGS}

test.obj:	test.c

.INCLUDE=$V/tools/makemeta
