#######################################################################

MAIN	=	OptCD

#######################################################################

CORE	=	cback.o		# c.o

PROGS	=	work:programs/

O	=	obj/

LIBS	=	lib:lc.lib lib:amiga.lib libl:Support.lib

OBJS	=	$ORecord.o

#######################################################################

CFLAGS=		-. -cafkqrt -j170 -j79 -iINCLUDEL: -iINCLUDESP:
LOPTS  =	NODEBUG SMALLCODE SMALLDATA Map $O$(MAIN).map
.c.o:
		lc $(CFLAGS) -o$O$*.o $*.c

#######################################################################

All:			$(PROGS)$(MAIN)

$(PROGS)$(MAIN):	$O$(MAIN).o $(OBJS)
			blink LIB:$(CORE) $O$(MAIN).o $(OBJS) TO $(PROGS)$(MAIN) LIB $(LIBS) $(LOPTS)

$O$(MAIN).o:		$(MAIN).c
$ORecord.o:		Record.c
