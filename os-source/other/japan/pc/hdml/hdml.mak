#
#	HDML EGConvert editor - (C) Copyright 1985-1993 by ERGOSOFT corp.
#	Makefile for building HDML library
#

# Source input directory
	SRCDIR	= source

# Object output directory (root)
	OBJDIR	= obj

# Object output directory
	OMODEL	= small

# Library output directory
	LIBDIR	= lib

# Compile model
	CMODEL	= /AS

# Include debuging information
	DEBUG	=

# Optimize flags
	OPTIMI	= /Od /Zi

# Warning level
	WLEVEL	= /W3

# Include directory
	INCDIR	= include
	EGCINC	= ..\egc\include

# Defines
    DEFINE	= /DEXP /DDOS_EGC /DHASPROTO

# Compile flags
	CFLAGS	= /c $(CMODEL) $(OPTIMI) $(WLEVEL)\
				/I$(INCDIR) /I$(EGCINC) $(DEFINE) /Fo$(OBJDIR)\$(OMODEL)\$*.obj

$(OBJDIR)\$(OMODEL)\EGBHDman.obj	:	$(SRCDIR)\EGBHDman.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGBHDclb.obj	:	$(SRCDIR)\EGBHDclb.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGBHDelb.obj	:	$(SRCDIR)\EGBHDelb.c
	cl $(CFLAGS) $(SRCDIR)\$*.c
	
$(OBJDIR)\$(OMODEL)\EGBHDerr.obj	:	$(SRCDIR)\EGBHDerr.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGBHDetc.obj	:	$(SRCDIR)\EGBHDetc.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGBHDmsc.obj	:	$(SRCDIR)\EGBHDmsc.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

# Library name
	LIBNAME	= HDML

$(LIBDIR)\$(OMODEL)\$(LIBNAME).lib :   \
$(OBJDIR)\$(OMODEL)\EGBHDelb.obj \
$(OBJDIR)\$(OMODEL)\EGBHDerr.obj \
$(OBJDIR)\$(OMODEL)\EGBHDetc.obj \
$(OBJDIR)\$(OMODEL)\EGBHDmsc.obj \
$(OBJDIR)\$(OMODEL)\EGBHDman.obj \
$(OBJDIR)\$(OMODEL)\EGBHDclb.obj
	del $(LIBDIR)\$(OMODEL)\$(LIBNAME).lib
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) $(OBJDIR)\$(OMODEL)\EGBHDelb;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGBHDerr;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGBHDetc;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGBHDmsc;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGBHDman;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGBHDclb;
	del $(LIBDIR)\$(OMODEL)\$(LIBNAME).bak
