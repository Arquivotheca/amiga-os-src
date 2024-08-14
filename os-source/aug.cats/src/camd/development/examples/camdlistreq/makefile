# =========================================================================
# Makefile for FilePilot using SAS C 6.3
# =========================================================================

# debugging flags for compiler and linker

OPTFLAGS  = opt optsize

#CDEBUG  = define NDEBUG=1 debug=fullflush
#LDEBUG  =
CDEBUG = nodebug  $(OPTFLAGS)
LDEBUG = stripdebug

CFLAGS	  = nolink ansi unsignedchars ignore=51 ignore=147 $(CDEBUG)

LISTLIB   = //lib/camdlist.lib

OBJS	  = $(MAINOBJS)

main: $(OBJS) makefile main.o $(LISTLIB)
        SLINK lib:c.o main.o noicons library lib:sc.lib lib:amiga.lib $(LISTLIB) to main


.c.o:
	SC $*.c $(CFLAGS)
