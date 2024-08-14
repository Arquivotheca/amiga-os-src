SYSTEMNAME = ReadJoyPort
SYSTEMVERSION = 1

RELEASEPATH = SoftPublic:SystemsAssurance/TestPrograms/
MODULENAME = lowlevel

OFILES = main.o gui.o joyport.o

CFILES = main.c gui.c joyport.c

AFILES =

MAKE = SMake

LTOOL = SLink
LLIBS = LIB:SCMIEEE.lib+LIB:SC.lib+LIB:amiga.lib+LIB:debug.lib+LIB:envoy.lib+LIB:ca_mw.lib
LFLAGS = VERBOSE BATCH NOICONS

ATOOL = casm
AFLAGS = -iInclude:

CTOOL = SC
CFLAGS = DEBUG=FULLFLUSH UTILLIB NOSTKCHK ERRREXX UCHAR STREQ DEFINE DEBUG
# CFLAGS = UTILLIB NOSTKCHK ERRREXX UCHAR STREQ OPTIMIZE

$(SYSTEMNAME).ld: $(OFILES) revision.o
    # Link executable code
    $(LTOOL) WITH $(SYSTEMNAME).with LIB $(LLIBS) $(LFLAGS) TO $(SYSTEMNAME).ld
    # Link stripped executable code
    $(LTOOL) WITH $(SYSTEMNAME).with LIB $(LLIBS) $(LFLAGS) TO $(SYSTEMNAME).ld.strip NODEBUG

release:
    # Bump revision
    bumprev $(SYSTEMVERSION) $(SYSTEMNAME)_rev
    # Touch revision module
    touch revision.c
    # Re-build
    $(MAKE)
    # Copy source code and documentation
    copy *.(c|a|h|i|rev|doc|doc.info) Makefile *.with TO $(RELEASEPATH)$(MODULENAME)/$(SYSTEMNAME)
    # Copy executable code
    copy $(SYSTEMNAME).ld.strip TO $(RELEASEPATH)$(MODULENAME)/$(SYSTEMNAME)/$(SYSTEMNAME)
    # Copy icon
    copy $(SYSTEMNAME).ld.strip.info TO $(RELEASEPATH)$(MODULENAME)/$(SYSTEMNAME)/$(SYSTEMNAME).info

.c.o:
    # Compile C source code
    $(CTOOL) $(CFLAGS) $(PRECOMPHDR) $*.c

.a.o:
    # Assemble assembly source code
    $(ATOOL) -a $*.a -o $*.o $(AFLAGS)
