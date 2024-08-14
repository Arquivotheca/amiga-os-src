# =========================================================================
# Makefile for FilePilot using SAS C 6.3
# =========================================================================

.c.o:
	SC $*.c $(CFLAGS)

# debugging flags for compiler and linker

OPTFLAGS  = opt optsize

#CDEBUG  = define NDEBUG=1 debug=fullflush
#LDEBUG  =
CDEBUG = nodebug  $(OPTFLAGS)
LDEBUG = stripdebug

CFLAGS	  = nolink ansi unsignedchars ignore=51 ignore=147 $(CDEBUG)

MAINOBJS  = selectconductor.o selectcluster.o listreq.o
MAINSRC   = selectconductor.c selectcluster.c listreq.c

OBJS	  = $(MAINOBJS)

main: $(OBJS) makefile main.o
	delete camdlist.lib
	oml camdlist.lib A $(OBJS)
        SLINK lib:c.o main.o $(OBJS) noicons library lib:sc.lib lib:amiga.lib to main
#	copy camdlist.lib sc:lib/
#	copy listreq.h i:include.supp/
#	copy camdlists.h i:include.supp/
#	hd:utils/autodoc >camdlist.doc -t4 -I -a -C $(MAINSRC)
