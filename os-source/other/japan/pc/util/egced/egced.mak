#
#       EGConvert Version 4.00 - (C) Copyright 1985-1993 by ERGOSOFT corp.
#       Makefile for building EGDict sequential file editor
#

# Source input directory
	SRCDIR  = source

# Object output directory (root)
	OBJDIR  = obj

# Include directory
	INCDIR  = include

# Library output directory
	EXEDIR  = exe

# Compile model
	CMODEL  = /AL

# Include debuging information
	DEBUG   =

# Optimize flags
	OPTIMI  = /Ox /Gs

# Warning level
	WLEVEL  = /W3

# Defines
    DEFINE      = /DCOMPLETE

# Compile flags
	CFLAGS  = /c    $(CMODEL) $(OPTIMI) $(WLEVEL) $(DEBUG) /I$(INCDIR) $(DEFINE) /Fo$(OBJDIR)\$*.obj

# Exec name
	EXENAME = EGCed

# Link flags
	LFLAG   = /NOL /F

$(OBJDIR)\Main.obj              :       $(SRCDIR)\Main.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Key.obj               :       $(SRCDIR)\Key.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\IO.obj                :       $(SRCDIR)\IO.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Init.obj              :       $(SRCDIR)\Init.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Csrmove.obj   :       $(SRCDIR)\Csrmove.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Box.obj               :       $(SRCDIR)\Box.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Select.obj    :       $(SRCDIR)\Select.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Display.obj   :       $(SRCDIR)\Display.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Command.obj   :       $(SRCDIR)\Command.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Comsub.obj    :       $(SRCDIR)\Comsub.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(EXEDIR)\$(EXENAME).exe :      $(OBJDIR)\Main.obj              \
							$(OBJDIR)\Init.obj              \
							$(OBJDIR)\Key.obj               \
							$(OBJDIR)\Select.obj    \
							$(OBJDIR)\Display.obj   \
							$(OBJDIR)\Comsub.obj    \
							$(OBJDIR)\IO.obj                \
							$(OBJDIR)\Box.obj               \
							$(OBJDIR)\Csrmove.obj   \
							$(OBJDIR)\Command.obj   \

	link $(LFLAG) @$(EXENAME).lnk,$(EXEDIR)\$(EXENAME),NUL,LLIBCE;
