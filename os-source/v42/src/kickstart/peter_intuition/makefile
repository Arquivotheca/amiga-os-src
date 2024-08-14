# Makefile for BMAKE

# UTILLIB?
# INTERNHFILES
# idebug.lib
# withfile

VERSION=42
MODULETYPE=kickstart
SYSTEM=intuition
ASMOPTS=OPTIMON 0xfdc0000 ADDSYM
CVERSION=6

SAS6OPTIONS   = RESOPT ERRREXX CODE=FAR DATA=FARONLY UCHAR CONSTLIB LIBCODE \
		    STREQ STRMER COMMENTNEST DBG=LINE GST=INCLUDE:all.gst \
		    MEMSIZE=HUGE NOSTKCHK NOICONS NOVERSION NOMINC \
		    OPT OPTPEEP OPTINLOCAL OPTSIZE OPTDEP=3 OPTRDEP=3 \
		    NOWVRET IGNORE 161 IGNORE 100

#SAS6OPTIONS   = RESOPT DATA=FARONLY PARM=REGISTERS UCHAR CONSTLIB LIBCODE\
#		    STREQ STRMER COMMENTNEST DBG=LINE GST=INCLUDE:all.gst\
#		    MEMSIZE=HUGE NOSTKCHK NOICONS NOVERSION NOMINC\
#		    OPT OPTPEEP OPTINLOCAL OPTSIZE OPTDEP=3 OPTRDEP=3

SUBSYSNAME=
SUBSYSTEMS=
AFILES=		iwork.asm intuitionface.asm \
		downcode.asm intuileap.asm lockstub.asm \
		mousebuttons.asm abs.asm hookface.asm aformat.asm \
		sysreqstack.asm xsysidata.asm classface.asm

INCLUDEDIR=
ASMOPTS=
CFILES= 	pubclasses.c classes.c rootclass.c gadclass.c propgadclass.c \
		imageclass.c ggclass.c icclass.c modclass.c \
		buttongclass.c strgadclass.c \
		frbuttonclass.c itexticlass.c fillrectclass.c frameiclass.c \
		vectorclass.c \
		ism.c iequeue.c ih.c ihfront.c isdefault.c pools.c \
		sidlewindow.c snowindow.c sgadget.c swsizedrag.c \
		sscreendrag.c smenu.c srequester.c sverify.c \
		sdmrtimeout.c \
		activate.c alert.c bhooks.c coercion.c \
		dclip.c \
		drawgadgets.c ezreq.c fonts.c format.c \
		gadgets.c \
		genv.c hitgadgets.c ihooks.c \
		idcmp.c ilock.c images.c init.c \
		iprefs.c isupport.c isupp2.c \
		layout.c \
		menu.c misc.c mouse.c \
		newlook.c \
		phooks.c pointer.c prop.c prefs.c pubscreen.c \
		rect.c requester.c \
		scale.c screens.c scsupp.c shooks.c sinput.c size.c \
		strings.c \
		thing.c timing.c varargs.c view.c \
		wbench.c windows.c wtags.c zoom.c autodocs.c


TEMPDIR=
HOMEDIR=
HOMEINCL=
LINKEROPTS=

# Public assembly include files
INFILES=	intuition.i intuitionbase.i screens.i preferences.i \
		cghooks.i sghooks.i classes.i classusr.i \
		imageclass.i gadgetclass.i icclass.i pointerclass.i \
		iobsolete.i


# Misc RCS files
OTHERFILES=	iprefs.h \
		intuall.h intuinternal.h ifunctions.h i.h \
		ism.h sc.h ibase.h ezreq.h vectorclass.h \
		newlook.h \
		intuall.i ibase.i vectorclass.i


NONRCS_FILES=setrev

# Public C include files
HFILES= 	intuition.h intuitionbase.h screens.h preferences.h \
		cghooks.h sghooks.h classes.h classusr.h \
		imageclass.h gadgetclass.h icclass.h pointerclass.h \
		iobsolete.h


#These are currently in intuition.with
#LINKEROPTS= SMALLCODE SMALLDATA ADDSYM  MAP $*.map XOHSFL PLAIN SWIDTH 16
#LINKERLIBS= idebug.lib LIB:rom.lib LIB:sc.lib LIB:debug.lib endcode.lib

LINKER=Slink

all:

abs.obj:	abs.asm intuition_rev.i

link:
		slink with intuition.with
		slink intuition.ld to intuition.ld.strip stripdebug

kickstart:
		kcustom change intuition.ld -wb.ld

%.omd:%.obj
	omd $*.obj $*.c >$*.omd

.include lib:makemeta
