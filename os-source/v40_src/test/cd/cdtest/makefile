SYSTEMNAME = cdtest
SYSTEMVERSION = 1

RELEASEPATH = SoftPublic:SystemsAssurance/TestPrograms/
MODULENAME = cd

OFILES = main.o event.o debug.o cd.o interrupt.o args.o verify.o simplerexx.o bsprintf.o

CFILES = main.c event.c debug.c cd.c interrupt.o args.c verify.c simplerexx.c

AFILES = bsprintf.a

MAKE = SMake

LTOOL = SLink
LLIBS = LIB:SCM.lib+LIB:SC.lib+LIB:amiga.lib+LIB:debug.lib+LIB:envoy.lib
LFLAGS = VERBOSE BATCH NOICONS

ATOOL = casm
AFLAGS = -iInclude:

CTOOL = SC
CFLAGS = DEBUG=FULLFLUSH UTILLIB ERRREXX UCHAR STREQ ADDSYM
# CFLAGS = UTILLIB NOSTKCHK ERRREXX UCHAR STREQ OPTIMIZE

$(SYSTEMNAME).ld: $(OFILES) revision.o
    $(LTOOL) WITH $(SYSTEMNAME).with LIB $(LLIBS) $(LFLAGS) TO $(SYSTEMNAME).ld
    $(LTOOL) WITH $(SYSTEMNAME).with LIB $(LLIBS) $(LFLAGS) TO $(SYSTEMNAME).ld.strip NODEBUG

release:
    bumprev $(SYSTEMVERSION) $(SYSTEMNAME)_rev
    touch revision.c
    $(MAKE)
    # Copy source code and documentation
    copy *.(c|a|h|i|rev|doc|doc.info) Makefile *.with TO $(RELEASEPATH)$(MODULENAME)/$(SYSTEMNAME)
    # Copy executable code
    copy $(SYSTEMNAME).ld.strip TO $(RELEASEPATH)$(MODULENAME)/$(SYSTEMNAME)/$(SYSTEMNAME)
    # Copy icon
    # copy $(SYSTEMNAME).ld.strip.info TO $(RELEASEPATH)$(MODULENAME)/$(SYSTEMNAME)/$(SYSTEMNAME).info

.c.o:
    $(CTOOL) $(CFLAGS) $*.c

.a.o:
    $(ATOOL) -a $*.a -o $*.o $(AFLAGS)
