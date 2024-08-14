
MODNAME	          = catcomp
VERSION		  = 40

REMOTE		  = V:aug/src/CatComp
REMOTE2		  = V:aug/bin/CatComp
REMOTE3		  = V:aug/doc/CatComp.doc

LD_FILE	          = $(MODNAME).ld
LD_STRIP          = $(MODNAME).ld.strip

C_COMPILER        = SC
C_COMPILER_OPTS   = RESOPT DATA=FARONLY PARM=REGISTERS UCHAR CONSTLIB\
		    STREQ STRMER COMMENTNEST DBG=SYMBOL GST=INCLUDE:all.gst\
		    ANSI MEMSIZE=HUGE NOSTKCHK NOICONS NOVERSION NOMINC\
		    OPT OPTPEEP OPTINLOCAL OPTSIZE OPTDEP=3 OPTRDEP=3 OPTSCHED

ASSEMBLER         = HX68 -a
ASSEMBLER_OPTS    = -csy -i include: -o

LINKER            = Slink
LINKER_OPTS	  = ADDSYM SMALLCODE SMALLDATA MAP $(MODNAME).map HLSXF
LINKER_OPTS_STRIP = ND

C_SOURCES   = catcomp.c texttable.c
ASM_SOURCES =
OBJECTS     = catcomp.o texttable.o
STARTUP     =
LIBS	    = LIB:debug.lib LIB:amiga.lib LIB:sc.lib


##############################################################################
# Build the stripped load file...
#
$(LD_STRIP):	$(LD_FILE)
		$(LINKER) FROM $(LD_FILE) TO $(LD_STRIP) $(LINKER_OPTS_STRIP)
		@Protect $(LD_STRIP) +p


##############################################################################
# Build the load file...
#
$(LD_FILE):	$(OBJECTS) $(LIBS)
		$(LINKER) FROM $(STARTUP) $(OBJECTS) TO $(LD_FILE) LIB $(LIBS) $(LINKER_OPTS)


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
catcomp.o: catcomp.c catcomp_rev.rev texttable.h
texttable.o: texttable.c texttable.h


##############################################################################
# Bump the version...
#
version:
	@BumpRev $(VERSION) $(MODNAME)_rev


##############################################################################
# Delete all object files
#
clean:
	@Delete \#?.o
	@Delete $(LD_FILE) $(LD_STRIP) $(MODNAME).map


##############################################################################
# Release stuff that should be...
#
headers:
	@protect $(REMOTE3) wrd
	@copy $(MODNAME).doc $(REMOTE3)
	@copy $(LD_STRIP) $(REMOTE2)


##############################################################################
# Release the current build to the server...
#
internal:
	@Delete $(REMOTE)/((\#?.(c|h|asm|i|ld|rev|doc|library))|lmkfile)
	@protect ((\#?.(c|h|asm|i|ld|rev|doc))|lmkfile|$(LD_STRIP)) +r quiet
	@copy \#?.(strip|ld|doc|library) \#?_rev.\#? $(REMOTE)
