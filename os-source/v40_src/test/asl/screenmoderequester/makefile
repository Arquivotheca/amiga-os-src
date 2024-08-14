# This Makefile sucks.
# But then, so does LMK.
#       jjszucs

TARGETBIN = screenmoderequester

TARGETVERSION = 38

CC = LC
CFLAGS = -d3

LN = BLink
LFLAGS = VERBOSE
LLIB = LIB:LC.lib LIB:amiga.lib LIB:debug.lib
LSTARTUP = LIB:c.o

screenmoderequester: screenmoderequester.o screenmoderequester_rev.o
    $(LN) $(LFLAGS) FROM $(LSTARTUP) $*.o $*_rev.o LIB $(LLIB) TO $@

screenmoderequester_rev.o: screenmoderequester.o
    bumprev $(TARGETVERSION) screenmoderequester_rev
    $(CC) screenmoderequester_rev
