#
#	EGConvert Version 4.00 - (C) Copyright 1985-1993 by ERGOSOFT corp.
#	Makefile for building EGConvert utilities
#

# EGConvert dicrectory
	EGCDIR	= ..\..\egc

# Source input directory
	SRCDIR	= source

# Object output directory (root)
	OBJDIR	= obj

# Library output directory
	LIBDIR	= lib

# Compile model
	CMODEL	= /AL

# Include debuging information
	DEBUG	= 

# Optimize flags
	OPTIMI	= /Ox /Gs

# Warning level
	WLEVEL	= /W3

# Include directory
	INCDIR	= $(EGCDIR)\include

# Defines
    DEFINE	= /DEXP /DDOS_EGC /DP_TYPE /DEGCVT

# Compile flags
	CFLAGS	= /c	$(CMODEL) $(OPTIMI) $(WLEVEL)\
					/I$(INCDIR) $(DEFINE) /Fo$(OBJDIR)\$*.obj

# EGConvert library directory
	ELIB	= $(EGCDIR)\lib\large\egc

# Parts library directory
	PLIB	= parts\lib\parts

# Output directory
	OUTDIR	= exe

# Stack
	STACK	= /ST:4096

# Link option
	LFLAG	= /NOL $(STACK)

$(OBJDIR)\EGCld.obj	:	$(SRCDIR)\EGCld.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OUTDIR)\EGCld.exe	:	$(OBJDIR)\EGCld.obj
	link $(LFLAG) $(OBJDIR)\$*.obj,$(OUTDIR)\$*.exe,NUL,$(ELIB)+$(PLIB);

$(OBJDIR)\EGCul.obj	:	$(SRCDIR)\EGCul.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OUTDIR)\EGCul.exe	:	$(OBJDIR)\EGCul.obj
	link $(LFLAG) $(OBJDIR)\$*.obj,$(OUTDIR)\$*.exe,NUL,$(ELIB)+$(PLIB);

$(OBJDIR)\EGCedic.obj	:	$(SRCDIR)\EGCedic.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OUTDIR)\EGCedic.exe	:	$(OBJDIR)\EGCedic.obj
	link $(LFLAG) $(OBJDIR)\$*.obj,$(OUTDIR)\$*.exe,NUL,$(ELIB)+$(PLIB);

$(OBJDIR)\EGCmexp.obj	:	$(SRCDIR)\EGCmexp.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OUTDIR)\EGCmexp.exe	:	$(OBJDIR)\EGCmexp.obj
	link $(LFLAG) $(OBJDIR)\$*.obj,$(OUTDIR)\$*.exe,NUL,$(ELIB)+$(PLIB);

$(OBJDIR)\EGCexch.obj	:	$(SRCDIR)\EGCexch.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OUTDIR)\EGCexch.exe	:	$(OBJDIR)\EGCexch.obj
	link $(LFLAG) $(OBJDIR)\$*.obj,$(OUTDIR)\$*.exe,NUL,$(ELIB)+$(PLIB);

$(OBJDIR)\EGCtbit.obj	:	$(SRCDIR)\EGCtbit.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OUTDIR)\EGCtbit.exe	:	$(OBJDIR)\EGCtbit.obj
	link $(LFLAG) $(OBJDIR)\$*.obj,$(OUTDIR)\$*.exe,NUL,$(ELIB)+$(PLIB);
