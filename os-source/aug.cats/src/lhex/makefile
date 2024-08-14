TARGET=lhex
O = .o
LHOBJS = lharc$(O) lhadd$(O) lhlist$(O) lhext$(O) header$(O)
HUFOBJS = append$(O) crcio$(O) dhuf$(O) extract$(O) huf$(O) \
          larc$(O) maketbl$(O) maketree$(O) shuf$(O) slide$(O) \
          util$(O) patmatch$(O)
# fastfile.o had CRC problems with some .lzh's
# AMIGAOBJS = amiga$(O) fastfile$(O)
AMIGAOBJS = amiga$(O)
LIBRARYOBJS = # LIB:debug.lib
LINKER = slink

OBJS = $(LHOBJS) $(HUFOBJS) $(AMIGAOBJS) $(LIBRARYOBJS)

#CFLAGS = dbg sf ignore all errorrexx def SYSTIME_HAS_NO_TM\

CFLAGS = def SYSTIME_HAS_NO_TM\
         def SYSV_SYSTEM_DIR def SIGHUP=SIGINT def TZSET def NOFTRUNCATE\
	 def NEED_INCREMENTAL_INDICATOR def AMIGA def LHEXTRACT

CC = sc


$(TARGET) : $(OBJS) Makefile
#	$(CC) LINK $(OBJS) programname $@ map mapxref maphunk
	$(CC) LINK $(OBJS) programname $@.withsym smallcode smalldata
	$(LINKER) $@.withsym to $@ ND 



version:
	bumprev 40 lhex_rev
	bumprev 40 lha_rev
	setdate amiga.o yesterday

clean:
	delete \#?.o lhex lhex.withsym

crcio$(O)   : crcio.c intrface.h slidehuf.h
extract$(O) : extract.c intrface.h
slide$(O)   : slide.c intrface.h

lhdir$(O)   : lhdir.c lhdir.h

lharc$(O)   : lharc.c lharc.h
lhadd$(O)   : lhadd.c lharc.h
lhlist$(O)  : lhlist.c lharc.h
lhext$(O)   : lhext.c lharc.h
header$(O)  : header.c lharc.h
append$(O)  : append.c lharc.h slidehuf.h
slide$(O)   : slide.c lharc.h
extract$(O) : extract.c lharc.h slidehuf.h


dhuf$(O)    : dhuf.c slidehuf.h
huf$(O)     : huf.c slidehuf.h
larc$(O)    : larc.c slidehuf.h 
maketbl$(O) : maketbl.c slidehuf.h 
maketree$(O): maketree.c slidehuf.h 
shuf$(O)    : shuf.c slidehuf.h 
slide$(O)   : slide.c slidehuf.h 
util$(O)    : util.c slidehuf.h

patmatch$(O): patmatch.c
amiga$(O)   : amiga.c
