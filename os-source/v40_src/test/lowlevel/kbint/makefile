SYSTEMNAME = KBInt
SYSTEMVERSION = 1

OFILES = main.o gui.o interrupt.o

CFILES = main.c gui.c interrupt.c

AFILES =

MAKE = SMake

LTOOL = SLink
LLIBS = LIB:SCM.lib+LIB:SC.lib+LIB:amiga.lib+LIB:debug.lib+LIB:envoy.lib
LFLAGS = VERBOSE BATCH NOICONS

ATOOL = casm
AFLAGS = -iInclude:

CTOOL = SC
CFLAGS = DEBUG=FULLFLUSH UTILLIB NOSTKCHK ERRREXX UCHAR STREQ DEFINE DEBUG
# CFLAGS = UTILLIB NOSTKCHK ERRREXX UCHAR STREQ OPTIMIZE

$(SYSTEMNAME).ld: $(OFILES) revision.o
    $(LTOOL) WITH $(SYSTEMNAME).with LIB $(LLIBS) $(LFLAGS) TO $(SYSTEMNAME).ld
    $(LTOOL) WITH $(SYSTEMNAME).with LIB $(LLIBS) $(LFLAGS) TO $(SYSTEMNAME).ld.strip NODEBUG

release:
    bumprev $(SYSTEMVERSION) $(SYSTEMNAME)_rev
    touch revision.c
    $(MAKE)

.c.o:
    $(CTOOL) $(CFLAGS) $(PRECOMPHDR) $*.c

.a.o:
    $(ATOOL) -a $*.a -o $*.o $(AFLAGS)
