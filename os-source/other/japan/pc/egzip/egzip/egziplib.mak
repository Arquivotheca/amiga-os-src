#/************************************************************************/
#/*                                                                      */
#/*    egzip.mak: Makefile EGZip sample program MS-C Ver6.0              */
#/*                                                                      */
#/*                         Version 1.00                                 */
#/*                                                                      */
#/*      (C) Copyright 1990 ERGOSOFT Corp.                               */
#/*      All Rights Reserved                                             */
#/*                                                                      */
#/*                         --- NOTE ---                                 */
#/*                                                                      */
#/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
#/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
#/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
#/*                                                                      */
#/************************************************************************/

    DSTDIR = small
    MODELS = /AS
    CDEBUG = #/Zi
    WINDOW = #/Zp /Gw
    OPTIMI = /Ox #als
    WARNIG = /W3
    DEFLIB = /Zl
    DSTOBJ = /Fo$(DSTDIR)
    STKCHK = /Gs
    CFLAGS = /c\
             $(STKCHK) $(WINDOW)\
             $(DEFLIB) $(CDEBUG)\
             $(MODELS) $(OPTIMI)\
             $(WARNIG) $(DSTOBJ)\$*.obj

    LDEBUG = /CO
	STACKS = #/ST:1000
    LFLAGS = $(LDEBUG) $(STACKS)
    SYSLIB = slibce
    ZIPLIB = egzip
   LINKLIB = $(SYSLIB)+$(DSTDIR)\$(ZIPLIB).lib
 TARGETEXE = $(DSTDIR)\egzip.exe
 TARGETOBJ = $(DSTDIR)\egzip.obj
 TARGETMAP = $(DSTDIR)\egzip.map

$(DSTDIR)\egzip.obj : egzip.c egzip.h
	cl $(CFLAGS) $*.c

$(DSTDIR)\egzipza.obj : egzipza.c egzip.h
	cl $(CFLAGS) $*.c

$(DSTDIR)\egzipaz.obj : egzipaz.c egzip.h
	cl $(CFLAGS) $*.c

$(DSTDIR)\stdutil.obj : stdutil.c egzip.h
	cl $(CFLAGS) $*.c

$(DSTDIR)\egzipio.obj : egzipio.c egzip.h
	cl $(CFLAGS) $*.c

$(DSTDIR)\egzipint.obj : egzipint.c egzip.h
	cl $(CFLAGS) $*.c

$(DSTDIR)\$(ZIPLIB).lib :   \
$(DSTDIR)\egzipza.obj \
$(DSTDIR)\egzipaz.obj \
$(DSTDIR)\stdutil.obj \
$(DSTDIR)\egzipio.obj \
$(DSTDIR)\egzipint.obj 
 cd $(DSTDIR)
 del $(ZIPLIB).lib
 lib $(ZIPLIB) EGZIPZA+EGZIPAZ+STDUTIL+EGZIPIO+EGZIPINT, lib$(ZIPLIB).lst;
 cd ..
 
$(DSTDIR)\egzip.exe : $(DSTDIR)\egzip.obj $(DSTDIR)\egzip.lib
 del $(DSTDIR)\egzip.exe
 link $(LFLAGS) $(TARGETOBJ),$(TARGETEXE),$(TARGETMAP),$(LINKLIB);
