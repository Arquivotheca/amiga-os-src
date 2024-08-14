
# APP, CFILES, and OFILES added by Toolmaker V1.19
#
# APP    = the application name
# CFILES = source files (may include SimpleRexx_tm.c and $(APP)_events.c)
# OFILES = object files (may include SimpleRexx_tm.o and $(APP)_events.o)
# HFILES = header files (may include SimpleRexx_tm.h)

APP	= db
CFILES	= $(APP).c $(APP)_tm.c dolist.c print.c bestid.c
OFILES	= $(APP).o $(APP)_tm.o dolist.o print.o bestid.o
HFILES	= $(APP).h $(APP)_text.h $(APP)_tm.h $(APP)_tm_text.h

# SAS 6.0 Makefile

CC	= sc
ASM	= asm
LINK	= slink
STARTUP	= lib:c.o
MYLIBS	=
CFLAGS	= streq strmer nominc nostkchk
AFLAGS	= -iInclude:
LFLAGS	= BATCH 
#DEFINE ___main=___tinymain


$(APP):	$(OFILES) Makefile
	$(LINK) FROM $(STARTUP) $(OFILES) \
	LIB $(MYLIBS) lib:sc.lib lib:amiga.lib \
	TO $(APP) ${LFLAGS}

$(APP).o:	 $(APP).c $(HFILES) Makefile

$(APP)_tm.o:	 $(APP)_tm.c $(HFILES) Makefile

$(APP)_events.o: $(APP)_events.c $(HFILES) Makefile

SimpleRexx_tm.o: SimpleRexx_tm.c $(HFILES) Makefile

dolist.o:	dolist.c Makefile

print.o: print.c Makefile

bestid.o: bestid.c Makefile

.c.o:
	$(CC) $(CFLAGS) $*.c

.asm.o:
	$(ASM) $(AFLAGS) $*.asm
