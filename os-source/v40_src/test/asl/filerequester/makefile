# This Makefile sucks.
# But then, so does LMK.
#       jjszucs

TARGETBIN = filerequester

TARGETVERSION = 38

CC = LC
CFLAGS = -d3

LN = BLink
LFLAGS = VERBOSE
LLIB = LIB:LC.lib LIB:amiga.lib LIB:debug.lib
LSTARTUP = LIB:c.o

filerequester: filerequester.o filerequester_rev.o
    $(LN) $(LFLAGS) FROM $(LSTARTUP) $*.o $*_rev.o LIB $(LLIB) TO $@

filerequester_rev.o: filerequester.o
    bumprev $(TARGETVERSION) filerequester_rev
    $(CC) filerequester_rev
