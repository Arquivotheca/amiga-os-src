#
# SAS C 6.1 Makefile for MIDI Capture utility
#
# APP    = the application name
# CFILES = source files (may include SimpleRexx_tm.c and $(APP)_events.c)
# OFILES = object files (may include SimpleRexx_tm.o and $(APP)_events.o)
# HFILES = header files (may include SimpleRexx_tm.h)

APP	= capture
CFILES	= $(APP).c $(APP)_tm.c $(APP)_events.c
OFILES	= $(APP).o $(APP)_tm.o $(APP)_events.o
HFILES	= $(APP).h $(APP)_text.h $(APP)_tm.h $(APP)_tm_text.h

LISTLIB = //lib/camdlist.lib

# Aztec C Makefile

CFLAGS	= nolink ansi unsignedchars multicharconstants debug=fullflush

.c.o:
	SC $*.c $(CFLAGS)

.a.o:
	SC $*.a nolink

all:	$(APP).o $(APP)_tm.o $(APP)

#nostdio

$(APP): $(OBJS)
        SLINK lib:c.o $(OFILES) noicons library $(LISTLIB) lib:sc.lib lib:amiga.lib to $(APP)
	

