#######################################################################

MAIN	=	ISOCD

#######################################################################

PROGS	=	/programs/

O	=	obj/

LIBS	=	lib:lc.lib lib:amiga2.0.lib libl:Support.lib

OBJS	=	$ODir.o $OFind.o $OISO.o $OOptimize.o $OOut.o $OSpace.o

#######################################################################

CFLAGS=		-. -cafkqrt -j170 -j79 -iINCLUDEL: -iINCLUDESP:
LOPTS  =	NOD SC SD Map $O$(MAIN).map
.c.o:
		lc $(CFLAGS) -o$O$*.o $*.c

#######################################################################

All:			$(PROGS)$(MAIN)

$(PROGS)$(MAIN):	$O$(MAIN).o $(OBJS)
			blink LIB:c.o $O$(MAIN).o $(OBJS) TO $(PROGS)$(MAIN) LIB $(LIBS) $(LOPTS)

$O$(MAIN).o:		$(MAIN).c
$ODir.o:		Dir.c
$OFind.o:		Find.c
$OISO.o:		ISO.c
$OOptimize.o:		Optimize.c
$OOut.o:		Out.c
$OSpace.o:		Space.c
