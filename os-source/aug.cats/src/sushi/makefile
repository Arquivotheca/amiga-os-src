# NOTE - this wmain.o is compiled -b0
HDRS = 

MYLIBS= LIB:debug.lib

APP=sushi

LOCAL = h:srcc/1.4/$(APP)
#REMOTE = V36:src/workbench/tools/$(APP)

LC = lc
ASM = HX68

OFILES  =	$(APP).o $(APP)a.o
CFILES	=	$(APP).c
AFILES  =	$(APP)a.asm
STARTUP =	wc.o wmainb0.o

# LFLAGS= DEFINE __main=__tinymain SMALLCODE SMALLDATA BATCH ND
LFLAGS= SMALLCODE BATCH ND
LCFLAGS = -b0 -cfistq -v -y -j73 -iINCLUDE:
AFLAGS = -iINCLUDE:


.SUFFIXES:
.SUFFIXES:	.o .c .h .asm .i

all:	$(APP)

$(APP):	$(OFILES) $(STARTUP)
	blink FROM $(STARTUP) $(OFILES) \
	LIB LIB:lc.lib LIB:amiga.lib $(MYLIBS) TO $(APP) ${LFLAGS}

.c.o:
	$(LC) $(LCFLAGS) $*.c
.asm.o:
	$(ASM) -a $*.asm $(AFLAGS) -o $*.o

$(APP).o:	$(APP).c
$(APP)a.o:	$(APP)a.asm

version:
	@bumprev 37 $(APP)_rev



