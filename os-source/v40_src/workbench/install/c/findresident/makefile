
MODNAME	          = findresident
VERSION		  = 40

REMOTE		  = V$(VERSION):src/workbench/install/c/findresident

LD_FILE	          = $(MODNAME).ld
LD_STRIP          = $(MODNAME).ld.strip

C_COMPILER        = SC
C_COMPILER_OPTS   = RESOPT DATA=FARONLY PARM=REGISTERS UCHAR CONSTLIB\
		    STREQ STRMER COMMENTNEST DBG=SYMBOL GST=INCLUDE:all.gst\
		    ANSI MEMSIZE=HUGE NOSTKCHK NOICONS NOVERSION NOMINC\
		    OPT OPTPEEP OPTINLOCAL OPTSIZE OPTDEP=3 OPTRDEP=3

LINKER            = Slink
LINKER_OPTS	  = ADDSYM SC MAP $(MODNAME).map HLSXF PLAIN SWIDTH 16
LINKER_OPTS_STRIP = ND

ASSEMBLER         = HX68 -a
ASSEMBLER_OPTS    = -csy -i include: -o

C_SOURCES   = findresident.c
ASM_SOURCES =
OBJECTS     = findresident.o
STARTUP     =
LIBS        = LIB:debug.lib LIB:amiga.lib LIB:sc.lib


##############################################################################
# Build the stripped load file...
#
$(LD_STRIP):	$(LD_FILE)
		$(LINKER) FROM $(LD_FILE) TO $(LD_STRIP) $(LINKER_OPTS_STRIP)


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
findresident.o: findresident.c $(MODNAME)_rev.rev


##############################################################################
# Bump the version...
#
version:
	@BumpRev $(VERSION) $(MODNAME)_rev


##############################################################################
# Delete all object files
#
clean:
	@delete \#?.o
	@delete $(LD_FILE) $(LD_STRIP) $(MODNAME).map


##############################################################################
# Release the current build to the server...
#
internal:
	@Delete $(REMOTE)/((\#?.(c|h|asm|i|ld|rev|doc|library))|lmkfile)
	@protect ((\#?.(c|h|asm|i|ld|rev|doc))|lmkfile|$(LD_STRIP)) +r quiet
	@copy \#?.(strip|ld|doc|library) \#?_rev.\#? $(REMOTE)
