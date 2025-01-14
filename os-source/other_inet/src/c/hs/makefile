# ---------------------------------------------------------------------------------
# Makefile for: handshake
#
# $Locker:  $
#
# $Id: makefile,v 1.1 91/05/09 16:02:21 bj Exp $
#
# $Revision: 1.1 $
#
# $Header: HOG:Other/inet/src/c/hs/RCS/makefile,v 1.1 91/05/09 16:02:21 bj Exp $
#
#-----------------------------------------------------------------------------------




#
# Makefile for HandShake using LMK and Lattice C 5.02 or greater 
# $Header: HOG:Other/inet/src/c/hs/RCS/makefile,v 1.1 91/05/09 16:02:21 bj Exp $
#
O  = o/
U  = h/
H  = ch/
Q  = ram:
SIZE  =
#OPTIM = -O
STCHK = -v
#DEBUG = -d5
NOSYM =
#
OBJS1 = $Oglobals.o $Oumain.o $OHandShake.o $Omenus.o $Odevice_init.o
OBJS2 = $Oserial_in.o $Oserial_out.o $Ovideo.o $Oblink.o $Oxprotocol.o $Oxprface.o
OBJS3 = $Okeypad.o $Ostringreq.o $Oerror.o $Olockedreq.o $Obeep.o $Oxprlink.o
OBJS4 = $Oescape.o $Odial.o $Otransfer.o $Otabreq.o $Oaboutreq.o $Ofonts.o
OBJS5 = $Ofunction_keys.o $Okermit.o $Oxmodem.o $Ofilereq.o $Orexx.o $Oxpraface.o
OBJS  = $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5)
#
MENUS1 = $Hphonemenu.h $Htitlemenu.h $Hsetupmenu.h $Hprojectmenu.h
MENUS2 = $Htransfermenu.h $Hterminalmenu.h $Harexxmenu.h
MENUS  = $(MENUS1) $(MENUS2)
#
HEADER1 = $Hasc.h $Hfunctions.h $Hbool.h       
HEADER2 = $Hterm.rpl $Hterm.gbl $Hglobals.h $Hterm.def $Htermall.h $Hvtf.h
HEADER3 = $Hterminc.h $Hminrexx.h $Hxproto.h $Hamiinc.h $Hamiinc.sym
HEADERS = $(HEADER1) $(HEADER2) $(HEADER3) $(MENUS)
#
FLAGS1 = $(DEBUG) $(STCHK) $(OPTIM) $(SIZE) -dTINY=1 -rr -ms -H$Hamiinc.sym -ccsfi -q$Q -o$O -i$H -iarp/include/
FLAGS2 = $(FLAGS1) -x
#
.c.o:
     @echo "Compiling $*.c"
     @echo "Flags: $(FLAGS2)"
     @lc >$*.lst $(FLAGS2) $*

.asm.o:
    @echo "Assembling $*.asm"
    asm >$*.lst -iinclude: -o$O$*.o $*.asm
#
.h.h:
    lcompact <$? >$@
#
.def.def:
    lcompact <$? >$@
#
.rpl.rpl:
    lcompact <$? >$@
#
.gbl.gbl:
    lcompact <$? >$@
#
# Main Target
#
HandShake:          $(HEADERS) $(OBJS)
    @echo "Linking $@"
    @-blink <with <(HandShake.with)
#BUFSIZE 5000
#DEFINE __main=__tinymain
VERBOSE
BATCH
TO       HandShake
ROOT     lib:cback.o,o/umain.o,o/HandShake.o,o/xprlink.o *
         o/device_init.o,o/serial_in.o o/xpraface.o *
         o/serial_out.o,o/globals.o,o/video.o,o/dial.o *
         o/transfer.o,o/menus.o,o/blink.o,o/fonts.o,o/keypad.o *
         o/error.o,o/tabreq.o,o/lockedreq.o,o/escape.o *

         o/function_keys.o,o/beep.o,o/filereq.o
         o/aboutreq.o
         o/rexx.o
         o/stringreq.o
         o/xmodem.o
         o/xprotocol.o
         o/xprface.o
         o/kermit.o
LIBRARY  arp/lib/a.lib *
         Work:HandShake/prosuite/colorwindow/colorwindow.lib *
         lib:lcr.lib #lib:amiga.lib
#NODEBUG
<
#
$Oglobals.o:       globals.c $(HEADERS)
     @echo "Compiling $*.c"
     @echo "Flags: $(FLAGS1)"
     @lc >$*.lst $(FLAGS1) $*
#
$Oumain.o:         umain.c
$Ofonts.o:         fonts.c
$OHandShake.o:     HandShake.c
$Odevice_init.o:   device_init.c
$Oserial_in.o:     serial_in.c
$Oserial_out.o:    serial_out.c
$Oblink.o:         blink.c
$Oescape.o:        escape.c
$Ovideo.o:         video.c
$Okeypad.o:        keypad.c
$Omenus.o:         menus.c $(MENUS)
$Ostringreq.o:     stringreq.c
$Oerror.o:         error.c
$Odial.o:          dial.c
$Ofunction_keys.o: function_keys.c
$Otransfer.o:      transfer.c
$Okermit.o:        kermit.c
$Oxmodem.o:        xmodem.c
$Oxprotocol.o:     xprotocol.c
$Oxprface.o:       xprface.c
     @echo "Compiling $*.c"
     @echo "Flags: $(DEBUG) $(STCHK) $(OPTIM) $(SIZE) -ms -H$Hamiinc.sym -ccsfi -q$Q -o$O -i$H -iarp/include/ "
     @lc >$*.lst $(DEBUG) $(STCHK) $(OPTIM) $(SIZE) -ms -H$Hamiinc.sym -ccsfi -q$Q -o$O -i$H -iarp/include/ $*
$Orexx.o:          rexx.c
$Otabreq.o:        tabreq.c
$Oaboutreq.o:      aboutreq.c
$Olockedreq.o:     lockedreq.c
$Ofilereq.o:       filereq.c
$Obeep.o :         beep.c

$Oxprlink.o:       xprlink.asm
$Oxpraface.o:      xpraface.asm

$Hfileio.h:        $Ufileio.h      
$Hasc.h:           $Uasc.h         
$Hfunctions.h:     $Ufunctions.h   
$Hbool.h:          $Ubool.h        
$Hterm.rpl:        $Uterm.rpl      
$Hterm.gbl:        $Uterm.gbl      
$Hglobals.h:       $Uglobals.h    
$Hphonemenu.h:     $Uphonemenu.h   
$Htitlemenu.h:     $Utitlemenu.h   
$Hsetupmenu.h:     $Usetupmenu.h   
$Hterm.def:        $Uterm.def      
$Hprojectmenu.h:   $Uprojectmenu.h 
$Htransfermenu.h:  $Utransfermenu.h
$Htermall.h:       $Utermall.h     
$Hvtf.h:           $Uvtf.h         
$Hterminalmenu.h:  $Uterminalmenu.h
$Harexxmenu.h:     $Uarexxmenu.h
$Hterminc.h:       $Uterminc.h
$Hminrexx.h:       $Uminrexx.h
$Hxproto.h:        $Uxproto.h
$Hamiinc.h:        $Uamiinc.h
#
$Hamiinc.sym:      amiinc.c $Hamiinc.h
    lc -ph -ih/ -o$H$*.sym $*
#
$OCvtParms.o:  CvtParms.c
#
CvtParms: $OCvtParms.o
    blink <with <(CvtParms.with)
#BUFSIZE 5000
VERBOSE
TO       CvtParms
ROOT     lib:c.o,$OCvtParms.o
LIBRARY  lib:lcr.lib lib:amiga.lib
<
    
#
backup: HandShake
    copy lmk\#? HandShake.1:Handshake
    copy \#?.c  HandShake.1:Handshake
    copy h/\#?  HandShake.1:Handshake/h
    copy doc/\#? HandShake.2:Handshake/doc
    copy o/\#?.o HandShake.2:Handshake/o
    copy HandShake HandShake.2:Handshake/HandShake
#
distribution:   HandShake
    execute <<(distribution)
blink Work:HandShake/HandShake to ram:Hndshk   NODEBUG NOICONS
blink Work:Handshake/CvtParms  to ram:CvtParms NODEBUG NOICONS
dir RAM: #comment
cd  RAM:  #comment
copy Work:HandShake/doc/\#? ""
rename HandShake.review hndshk.rev
rename HandShake.doc hndshk.doc
rename HandShake_ARexx.doc arexx.doc
rename HandShake.info hndshk.info
rename HandShake.parms hndshk.prm
arc a HDSK \#?
<
