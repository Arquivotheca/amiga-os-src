
MODNAME	          = mpegplayer
VERSION		  = 40

REMOTE		  = V$(VERSION):src/kickstart/mpegplayer

LD_FILE	          = $(MODNAME).ld
LD_STRIP          = $(MODNAME).ld.strip

C_COMPILER        = SC
C_COMPILER_OPTS   = RESOPT DATA=FARONLY PARMS=REGISTERS UCHAR CONSTLIB\
		    STREQ STRMER COMMENTNEST DBG=LINE CPU=68020\
		    ANSI MEMSIZE=HUGE NOSTKCHK NOICONS NOVERSION NOMINC\
		    GST INCLUDE:all.gst IGNORE 51,162,104\
		    OPT OPTPEEP OPTINLOCAL OPTSIZE OPTDEP=3 OPTRDEP=3 OPTSCHED

ASSEMBLER         = HX68 -a
ASSEMBLER_OPTS    = -csy -i include: -o

LINKER            = Slink
LINKER_OPTS	  = SC SD MAP $(MODNAME).map HLSXF PLAIN SWIDTH 16
LINKER_OPTS_STRIP = ND

C_SOURCES   = init.c utils.c eventloop.c tracks.c diskinfo.c text.c credits.c\
	      play.c cdg.c tasks.c spliner.c thumbnail.c

ASM_SOURCES = mpegplayer_lib.asm compact24.asm compact31.asm nprintf.asm\
              splines.asm halvechunky.asm endcode.asm

OBJECTS     = mpegplayer_lib.o init.o utils.o eventloop.o nprintf.o\
	      diskinfo.o tracks.o text.o credits.o cdg.o play.o splines.o\
	      tasks.o spliner.o halvechunky.o thumbnail.o compact24.o\
	      compact31.o background.o icons.o staff.o cdgicons.o arrows.o\
	      endcode.o

STARTUP     =
LIBS	    = LIB:sc.lib LIB:amiga.lib LIB:debug.lib


##############################################################################
# Build the stripped load file...
#
$(LD_STRIP):	$(LD_FILE)
		$(LINKER) FROM $(LD_FILE) TO $(LD_STRIP) $(LINKER_OPTS_STRIP)


##############################################################################
# Build the load file...
#
$(LD_FILE):	$(OBJECTS) $(LIBS)

		$(LINKER) <with <
FROM $(STARTUP) $(OBJECTS) TO $(LD_FILE) LIB $(LIBS) $(LINKER_OPTS)
<


##############################################################################
# Default rules...
#
.c.o:
	$(C_COMPILER) $(C_COMPILER_OPTS) $*.c

.asm.o:
	$(ASSEMBLER) $*.asm $(ASSEMBLER_OPTS) $*.o


##############################################################################
# All dependencies...
#
mpegplayer_lib.o: mpegplayer_lib.asm mpegplayerbase.i $(MODNAME)_rev.rev
utils.o: utils.c utils.h mpegplayerbase.h
eventloop.o: eventloop.c eventloop.h tracks.h diskinfo.h defs.h mpegplayerbase.h text.h
tracks.o: tracks.c tracks.h defs.h mpegplayerbase.h diskinfo.h
diskinfo.o: diskinfo.c diskinfo.h defs.h mpegplayerbase.h
compact24.o: compact24.asm
compact31.o: compact31.asm
nprintf.o: nprintf.asm
text.o: text.c text.h mpegplayerbase.h defs.h
credits.o: credits.c credits.h mpegplayerbase.h defs.h diskinfo.h
play.o: play.c play.h mpegplayerbase.h cdg.h diskinfo.h defs.h
splines.o: splines.asm
tasks.o: tasks.c tasks.h mpegplayerbase.h
spliner.o: spliner.c spliner.h mpegplayerbase.h
cdg.o: cdg.c cdg.h mpegplayerbase.h
halvechunky.o: halvechunky.asm
thumbnail.o: thumbnail.c thumbnail.h mpegplayerbase.h utils.h tasks.h
endcode.o: endcode.asm

## Because of a bug in the SAS/C 6.50 peephole optimizer, this module
## must be compiled with the optimizer turned off
init.o: init.c mpegplayerbase.h
	SC RESOPT DATA=FARONLY PARMS=REGISTERS UCHAR CONSTLIB\
	   STREQ STRMER COMMENTNEST DBG=LINE CPU=68020\
	   ANSI MEMSIZE=HUGE NOSTKCHK NOICONS NOVERSION NOMINC\
	   GST INCLUDE:all.gst IGNORE 51,162,104\
	   OPT NOOPTPEEP OPTINLOCAL OPTSIZE OPTDEP=3 OPTRDEP=3 OPTSCHED init.c


background.o: background.pic
        @enforcer OFF >NIL:
        sbox -XPR $>.pic T:dbox.tmp
        d2o T:dbox.tmp _$> $>.o -h "_NOMERGE"
        @Delete T:dbox.tmp quiet
        @RUN >NIL: enforcer ON

icons.o: icons.pic
        @enforcer OFF >NIL:
        sbox -XPCR $>.pic T:dbox.tmp
        d2o T:dbox.tmp _$> $>.o -h "_NOMERGE"
        @Delete T:dbox.tmp quiet
        @RUN >NIL: enforcer ON

staff.o: staff.pic
        @enforcer OFF >NIL:
        sbox -XPCR $>.pic T:dbox.tmp
        d2o T:dbox.tmp _$> $>.o -h "_NOMERGE"
        @Delete T:dbox.tmp quiet
        @RUN >NIL: enforcer ON

cdgicons.o: cdgicons.pic
        @enforcer OFF >NIL:
        sbox -XPR $>.pic T:dbox.tmp
        d2o T:dbox.tmp _$> $>.o -h "_NOMERGE"
        @Delete T:dbox.tmp quiet
        @RUN >NIL: enforcer ON

arrows.o: arrows.pic
        @enforcer OFF >NIL:
        sbox -XPCR $>.pic T:dbox.tmp
        d2o T:dbox.tmp _$> $>.o -h "_NOMERGE"
        @Delete T:dbox.tmp quiet
        @RUN >NIL: enforcer ON


##############################################################################
# Bump the version...
#
version:
	@BumpRev $(VERSION) $(MODNAME)_rev


##############################################################################
# Release stuff that should be...
#
headers:
	@autodoc >$(MODNAME).doc autodocs.asm
#	@copy $(MODNAME).doc V$(VERSION):doc
#	@delete $(MODNAME).doc quiet
	@protect V$(VERSION):include/internal/cdui.[ih] wrd quiet
	@copy cdui.[ih] V$(VERSION):include/internal quiet
	@protect V$(VERSION):lib/sfd/cdui_lib.sfd wrd
	@copy cdui_lib.sfd V$(VERSION):lib/sfd


##############################################################################
# Delete all object files
#
clean:
	@Delete (\#?.o|\#?.o.\#?|$(MODNAME).doc|$(MODNAME).ld\#?|$(MODNAME).map\#?)


##############################################################################
# Release the current build to the server...
#
internal:
	@Delete $(REMOTE)/((\#?.(c|h|asm|i|ld|rev|doc|library)\#?)|lmkfile)
	@protect ((\#?.(c|h|asm|i|ld|rev|doc))|lmkfile|$(LD_STRIP)) +r quiet
	@copy \#?.(strip|ld|doc|library)\#? \#?_rev.\#? $(REMOTE)
