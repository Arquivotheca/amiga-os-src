
# jimm's small makefile
V=      /usr/commodore/amiga/V
LIB=        $(V)/internal/lib
SSTART=     20000
STARTUP=    $(V)/internal/lib/startup.obj
MAKEFILE=   makefile
MYLIBS=     $(LIB)/debug.lib

SUBSYSNAME= suck

#LFLAGS=    map=$*.lmap xref=$*.xref
CFILES=     $(SUBSYSNAME).c sgad.c chunk.c
OFILES=     $(SUBSYSNAME).obj sgad.obj chunk.obj

all:        $(SUBSYSNAME).ld
#$(OFILES): $(SUBSYSNAME).h

.INCLUDE=$(V)/tools/makemeta

down:
	mkdown $(CFILES) $(SUBSYSNAME).h  $(SUBSYSNAME).with  > down

tags:	$(CFILES) $(SUBSYSNAME).h
	xtags $(CFILES) $(SUBSYSNAME).h

