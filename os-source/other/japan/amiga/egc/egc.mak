#
#       EGConvert Version 4.00 - (C) Copyright 1985-1993 by ERGOSOFT corp.
#       Makefile for building EGConvert library
#

# Source input directory
	SRCDIR = source

# Object output directory (root)
	OBJDIR = obj

# Object output directory
	OMODEL= small

# Library output directory
	LIBDIR= lib

# Compile model
	CMODEL=  /AS

# Include debuging information
	DEBUG=

# Optimize flags
	OPTIMI= /Od /Zi

# Warning level
	WLEVEL= /W3

# Include directory
	INCDIR= include

# Defines
	DEFINE= /DEXP /DDOS_EGC /DP_TYPE /DEGCVT /DPMSLT /DBHREAD

# Compile flags
	CFLAGS= /c $(CMODEL) $(OPTIMI) $(WLEVEL) /I$(INCDIR) $(DEFINE) /Fo$(OBJDIR)\$(OMODEL)\$*.obj

# File I/O system source
	FIOS=   EGDFIOS
	      

$(OBJDIR)\$(OMODEL)\EGCldmlm.obj        :       $(SRCDIR)\EGCldmlm.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCldmls.obj        :       $(SRCDIR)\EGCldmls.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCktok.obj :       $(SRCDIR)\EGCktok.c
	cl $(CFLAGS) $(SRCDIR)\$*.c
	
$(OBJDIR)\$(OMODEL)\EGCdios.obj :       $(SRCDIR)\EGCdios.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGChios.obj :       $(SRCDIR)\EGChios.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCmios.obj :       $(SRCDIR)\EGCmios.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCfios.obj :       $(SRCDIR)\EGCfios.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCmdic.obj :       $(SRCDIR)\EGCmdic.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCdsys.obj :       $(SRCDIR)\EGCdsys.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCrdic.obj :       $(SRCDIR)\EGCrdic.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCvers.obj :       $(SRCDIR)\EGCvers.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCctype.obj        :       $(SRCDIR)\EGCctype.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCrcvt.obj :       $(SRCDIR)\EGCrcvt.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\EGCccvt.obj :       $(SRCDIR)\EGCccvt.c
	cl $(CFLAGS) $(SRCDIR)\$*.c

$(OBJDIR)\$(OMODEL)\$(FIOS).obj :       $(SRCDIR)\$(FIOS).asm
	masm /MX /I$(INCDIR) $(SRCDIR)\$*.asm, $(OBJDIR)\$(OMODEL)\$*.obj;

# Library name
	LIBNAME=        egc

$(LIBDIR)\$(OMODEL)\$(LIBNAME).lib :   \
$(OBJDIR)\$(OMODEL)\EGCktok.obj \
$(OBJDIR)\$(OMODEL)\EGCdios.obj \
$(OBJDIR)\$(OMODEL)\EGCdsys.obj \
$(OBJDIR)\$(OMODEL)\EGChios.obj \
$(OBJDIR)\$(OMODEL)\EGCmios.obj \
$(OBJDIR)\$(OMODEL)\EGCfios.obj \
$(OBJDIR)\$(OMODEL)\EGCldmlm.obj \
$(OBJDIR)\$(OMODEL)\EGCldmls.obj \
$(OBJDIR)\$(OMODEL)\EGCrcvt.obj \
$(OBJDIR)\$(OMODEL)\EGCccvt.obj \
$(OBJDIR)\$(OMODEL)\EGCrdic.obj \
$(OBJDIR)\$(OMODEL)\EGCmdic.obj \
$(OBJDIR)\$(OMODEL)\EGCvers.obj \
$(OBJDIR)\$(OMODEL)\EGCctype.obj \
$(OBJDIR)\$(OMODEL)\$(FIOS).obj 
	del $(LIBDIR)\$(OMODEL)\$(LIBNAME).lib
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) $(OBJDIR)\$(OMODEL)\EGCdsys;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCCCVT;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCLDMLm;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCLDMLs;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCMIOS;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCRCVT;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCvers;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGChios;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCfios;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCctype;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCRDIC;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCDIOS;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCMDIC;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\EGCKTOK;
	lib /NOL $(LIBDIR)\$(OMODEL)\$(LIBNAME) +$(OBJDIR)\$(OMODEL)\$(FIOS);
	del $(LIBDIR)\$(OMODEL)\$(LIBNAME).bak
