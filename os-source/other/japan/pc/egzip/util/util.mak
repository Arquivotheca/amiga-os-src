#/************************************************************************/
#/*                                                                      */
#/*    util.mak: Makefile EGZIP utility program MS-C Ver5.1              */
#/*                                                                      */
#/*      (C) Copyright 1990-1991 ERGOSOFT Corp.                          */
#/*      All Rights Reserved                                             */
#/*                                                                      */
#/*                         --- NOTE ---                                 */
#/*                                                                      */
#/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
#/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
#/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
#/*                                                                      */
#/************************************************************************/

    DSTDIR = tool
    MODELS = /AL
    CDEBUG = /Zi
    WINDOW = #/Zp /Gw
    OPTIMI = /Ox #als
    WARNIG = /W3
    DSTOBJ = /Fo$(DSTDIR)
    DSTEXE = /Fe$(DSTDIR)
    STKCHK = /Gs
    CFLAGS = /c \
             $(STKCHK) $(WINDOW)\
             $(DEFINE) $(CDEBUG)\
             $(MODELS) $(OPTIMI)\
             $(WARNIG)\
             $(DSTOBJ)\$*.obj

    LDEBUG = #/CO
    STACKS = #/ST:1000
    LFLAGS = $(LDEBUG) $(STACKS)
    SYSLIB = llibce
   LNKLIB = $(SYSLIB)

$(DSTDIR)\makezipm.obj : makezipm.c 
	cl $(CFLAGS) $*.c 

$(DSTDIR)\makezipm.exe : $(DSTDIR)\$*.obj
   link $(LFLAGS) $(DSTDIR)\$*.obj,$(DSTDIR)\$*.exe,$(DSTDIR)\$*.map,$(LNKLIB);

$(DSTDIR)\dispzipm.obj: dispzipm.c 
	cl $(CFLAGS) $*.c 

$(DSTDIR)\dispzipm.exe: $(DSTDIR)\$*.obj
   link $(LFLAGS) $(DSTDIR)\$*.obj,$(DSTDIR)\$*.exe,$(DSTDIR)\$*.map,$(LNKLIB);

$(DSTDIR)\dmpzl.obj: dmpzl.c
	cl $(CFLAGS) $*.c 

$(DSTDIR)\dmpzl.exe: $(DSTDIR)\$*.obj
   link $(LFLAGS) $(DSTDIR)\$*.obj,$(DSTDIR)\$*.exe,$(DSTDIR)\$*.map,$(LNKLIB);

$(DSTDIR)\dmpvl.obj : dmpvl.c
	cl $(CFLAGS) $*.c 

$(DSTDIR)\dmpvl.exe : $(DSTDIR)\$*.obj
   link $(LFLAGS) $(DSTDIR)\$*.obj,$(DSTDIR)\$*.exe,$(DSTDIR)\$*.map,$(LNKLIB);


