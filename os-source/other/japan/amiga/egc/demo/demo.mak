#
#	EGConvert Version 4.00 - (C) Copyright 1993 by ERGOSOFT corp.
#	Makefile for building EGConvert testing demo
#

# EGConvert dicrectory
	EGCDIR	= ..

# Utility parts directory
	PARTDIR	= ..\..\util\egcu\parts

# Source input directory
	SRCDIR	= source

# Object output directory (root)
	OBJDIR	= obj

# Compile model
	CMODEL	= /AL

# Include debuging information
	DEBUG	=

# Optimize flags
	OPTIMI	= /Od /Zi

# Warning level
	WLEVEL	= /W3

# Include directory
	INCDIR	= $(EGCDIR)\include

# Defines
    DEFINE	= /DEXP /DDOS_EGC /DP_TYPE /DEGCVT /DSIN /DALLSRC

# Compile flags
	CFLAGS	= /c	$(CMODEL) $(OPTIMI) $(WLEVEL) $(DEBUG)\
					/I$(INCDIR) $(DEFINE) /Fo$(OBJDIR)\$*.obj

# EGConvert library directory
	ELIB	= $(EGCDIR)\lib\large\egc

# Parts library directory
	PLIB	= $(PARTDIR)\lib\parts

# Output directory
	OUTDIR	= exe

# Stack
#	STACK	= /ST:4096
	STACK	= /ST:10240

# Link option
	LFLAG	= /NOL $(STACK) /CO

# Exec name
	EXENAME	= ct

$(OBJDIR)\$(EXENAME).obj	:	$(SRCDIR)\$(EXENAME).c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OUTDIR)\$(EXENAME).exe	:	$(OBJDIR)\$(EXENAME).obj $(ELIB).lib
	link $(LFLAG) $(OBJDIR)\$*.obj,$(OUTDIR)\$*.exe,NUL,$(ELIB)+$(PLIB);
