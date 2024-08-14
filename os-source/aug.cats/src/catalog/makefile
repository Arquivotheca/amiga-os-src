
APP	=helloworld

LC	= lc
ASM	= HX68

#MYLIBS	= LIB:debug.lib
LIBS	= LIB:lc.lib LIB:amiga.lib
STARTUP = LIB:c.o

LFLAGS	= SMALLCODE BATCH ND
LCFLAGS = -b1 -cfistq -v -y -j73 -iINCLUDE:
AFLAGS 	= -iINCLUDE:

# $(APP)_strings.o created by CatComp from .cd file
OFILES  =	$(APP).o catalog.o getcatalogstr.o
CFILES	=	$(APP).c catalog.c
AFILES  =	getcatalogstr.asm


$(APP):	$(OFILES) $(STARTUP)
	blink FROM $(STARTUP) $(OFILES) \
	LIB $(LIBS) $(MYLIBS) TO $(APP) $(LFLAGS)

.c.o:
	$(LC) $(LCFLAGS) $*.c
.asm.o:
	$(ASM) -a $*.asm $(AFLAGS) -o $*.o

