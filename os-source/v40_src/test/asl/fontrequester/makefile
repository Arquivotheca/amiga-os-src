# This Makefile sucks.
# But then, so does LMK.
#       jjszucs

TARGETBIN = fontrequester

TARGETVERSION = 38

CC = LC
CFLAGS = -d3

LN = BLink
LFLAGS = VERBOSE
LLIB = LIB:LC.lib LIB:amiga.lib LIB:debug.lib
LSTARTUP = LIB:c.o

fontrequester: fontrequester.o fontrequester_rev.o
    $(LN) $(LFLAGS) FROM $(LSTARTUP) $*.o $*_rev.o LIB $(LLIB) TO $@

fontrequester_rev.o: fontrequester.o
    bumprev $(TARGETVERSION) fontrequester_rev
    $(CC) fontrequester_rev
