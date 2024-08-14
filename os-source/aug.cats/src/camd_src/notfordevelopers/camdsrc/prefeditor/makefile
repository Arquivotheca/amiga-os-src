
# APP, CFILES, and OFILES added by Toolmaker V1.13
#
# APP    = the application name
# CFILES = source files (may include SimpleRexx_tm.c and $(APP)_events.c)
# OFILES = object files (may include SimpleRexx_tm.o and $(APP)_events.o)
# HFILES = header files (may include SimpleRexx_tm.h)

APP	= CAMDPrefs
CFILES	= $(APP).c $(APP)_tm.c $(APP)_events.c
OFILES	= $(APP).o $(APP)_tm.o $(APP)_events.o
HFILES	= $(APP).h $(APP)_text.h $(APP)_tm.h $(APP)_tm_text.h

# Aztec C Makefile

#CFILES	= $(APP).c $(APP)_tm.c
#OFILES	= $(APP).o $(APP)_tm.o
#MYLIBS	=

CC	= cc
ASM	= as
LINK	= ln
CFLAGS	= -ppac0t -wp -bs -hi ram:Lib/include.pre -qf
AFLAGS	=
LFLAGS	= -g
REXXO   =
MYLIBS  =

all:	$(APP).o $(APP)_tm.o $(APP)

.c.o:
	$(CC) $(CFLAGS) -o $*.o $*.c

.asm.o:
	$(ASM) $(AFLAGS) -o $*.o $*.asm

$(APP):	$(OFILES) $(REXXO) Makefile
	$(LINK) -o MidiPorts $(LFLAGS) $(OFILES) $(REXXO) -lx -lc $(MYLIBS)
