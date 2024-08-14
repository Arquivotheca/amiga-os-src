#
#	EGConvert Version 4.00 - (C) Copyright 1985-1993 by ERGOSOFT corp.
#	Makefile for building parts library used by EGCutility
#

# EGConvert dicrectory
	EGCDIR	= ..\..\..\egc

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

$(OBJDIR)\Load.obj	:	$(SRCDIR)\Load.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Unload.obj	:	$(SRCDIR)\Unload.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Dicsel.obj	:	$(SRCDIR)\Dicsel.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Marge.obj	:	$(SRCDIR)\Marge.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\Abort.obj	:	$(SRCDIR)\Abort.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

# Library name
	LIBNAME	= parts

$(LIBDIR)\$(LIBNAME).lib :   \
$(OBJDIR)\Load.obj \
$(OBJDIR)\Unload.obj \
$(OBJDIR)\Dicsel.obj \
$(OBJDIR)\Marge.obj \
$(OBJDIR)\Abort.obj
	del $(LIBDIR)\$(LIBNAME).lib
	lib /NOL $(LIBDIR)\$(LIBNAME) $(OBJDIR)\Dicsel;
	lib /NOL $(LIBDIR)\$(LIBNAME) +$(OBJDIR)\Marge;
	lib /NOL $(LIBDIR)\$(LIBNAME) +$(OBJDIR)\Load;
	lib /NOL $(LIBDIR)\$(LIBNAME) +$(OBJDIR)\Unload;
	lib /NOL $(LIBDIR)\$(LIBNAME) +$(OBJDIR)\Abort;
	del $(LIBDIR)\$(LIBNAME).bak
