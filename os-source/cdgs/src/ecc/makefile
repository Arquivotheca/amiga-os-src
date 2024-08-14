#
# $Id: makefile,v 1.2 93/02/10 09:38:10 havemose Exp Locker: havemose $
# Makefile for ECC project.
#
# The files: ecc.c and ecctab.c are the only files that should go into
# a ROM build.
#
#DBG=DEBUG=SYMBOL
DBG=NODEBUG
CFLAGS= UNSCHAR  DATA=FAR STRMERGE $(DBG) CPU=68020  NOSTKCHK OPTIMIZE OPTIMIZERDEPTH=5
OFILES= main.o ecc.o ecctab.o readdat.o #eccdown.o
SERVER= CDGS:src/cd/ecc.ah

.c.o:
	SC $(CFLAGS) $*.c
.asm.o:
        asm $(AFLAGS) $*.asm

ecc:	$(OFILES) makefile ecc.h
	slink FROM LIB:c.o $(OFILES) TO ecc \
	LIBRARY LIB:sc.lib LIB:amiga.lib VERBOSE ADDSYM

main.c:	ecc.h
ecc.o:	ecc.c ecc.h

server:
	echo "Make sure to check all file in"
	#dir \#?.c
	#dir \#?.doc
	#dir \#?.h
	copy \#?.c $(SERVER)	clone
	copy \#?.doc $(SERVER)	clone
	copy \#?.h $(SERVER)	clone
	copy makefile $(SERVER)	clone
	copy ecc.o ecctab.o $(SERVER) clone
