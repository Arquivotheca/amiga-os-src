################################################################################
# GNU makefile, default version, for SAS/C V6.2
#
# Usage: Make ProjectName  ; without dependencies, you'd best have ProjectName.c
#    or: Make Module.o     ; compiles a single module
#
############################ development environment ############################
# -csi: one copy of string, no multiincludes; 
# -t smallcode, smalldata ('t'ypical 'good' opts from SAS)
# -j87e wrong number of args to function is now an error, not a warning
#
CC       := sc:c/sc
AS       := casm -a
LN       := sc:c/slink
STARTUP  := c
OPTFLAGS := stringmerge nminc startup=$(STARTUP) error=87 idir=include: OPT OptInL OptInLocal nostkchk nolink noversion mccons
NOOPTFLG := stringmerge nminc startup=$(STARTUP) error=87 idir=include: nolink noversion mccons
CFLAGS   := $(NOOPTFLG)
LDLIBS   := sc sd stripdebug define ___main=___tinymain
LOADLIBES := lib:sc.lib lib:amiga.lib lib:ErrReport.lib lib:debug.lib lib:wb2cli.o
OBJ      = $@_rev.o $(filter %.o,$^)
ASFLAGS  := -o
PROGS    := DeleteNV GetCopyNV GetNVInfo GetNVList SetNVProtection StoreNV WipeNV StuffNV

################### GNU Make configuration #######################################

# don't consider modifying the makefile
.PHONY: makefile
.PHONY: all

# kill plethora of predefined suffix rules
.SUFFIXES :

# define new suffix rules
.SUFFIXES : %.c %.o %.asm

######################## DEFAULT RULES ###########################################
# Default rule for making *.asm into *.o (C.A.P.E. bias) -------------------------
%.o : %.asm
    $(AS) $*.asm $(ASFLAGS) $@

# Default rule for making *.c into *.o ------------------------------------------
# To compile a single module, say MAKE FOO.O
%.o : %.c
    $(CC) $(CFLAGS) objname=$@ $*

# Default Link Rule -------------------------------------------------------------
# Default rule for turning *.o's into PROGNAME  (you said MAKE PROGNAME, right?)
# Note that by default, ALL *.o's in the directory are linked into the program!
# Comment out the blink line and uncomment the line below it to use just one .o file.
# Note:  this breaks if there aren't any .o files in the current directory.
% : %.o
    $(revise)
    $(LN) from lib:$(STARTUP).o $(OBJ) to $@ LIB $(LOADLIBES) $(LDLIBS)
#   blink from lib:c.o $@_rev.o $^ to $@ LIB $(LOADLIBES) $(LDLIBS)


##################### DEPENDENCIES ###################################################
# Dependencies and interrelations of .o files.  This is how Make knows what
# it needs to make in a multi-module project.  Example for foo.c, foo2.c, foo.h, and
# foo3.asm, linking to program FOO:
# foo : foo2.o foo3.o foo.h

all : $(PROGS)

SetNVProtection : setnvprotection.o xstrtoulong.o

df0 : $(PROGS)
    copy $(PROGS) df0: clone

df1 : $(PROGS)
    copy $(PROGS) df1: clone

envoy : $(PROGS)
    copy $(PROGS) SoftPublic:SystemsAssurance/Lockhart/NVLib/bin clone
    copy #?.(c|h|asm) makefile SoftPublic:SystemsAssurance/Lockhart/NVLib/src clone
    copy #?.doc SoftPublic:SystemsAssurance/Lockhart/NVLib/doc clone
    copy rexx/#? SoftPublic:SystemsAssurance/Lockhart/NVLib/rexx clone

cdgs : $(PROGS)
    copy $(PROGS) f:cdgs/TestCode/nonvolatile.library/bin clone
    copy #?.(c|h|asm) makefile f:cdgs/TestCode/nonvolatile.library/src clone
    copy #?.doc f:cdgs/TestCode/nonvolatile.library/doc clone
    copy rexx/#? f:cdgs/TestCode/nonvolatile.library/rexx clone

df0bin : $(PROGS)
    copy $(PROGS) df0:bin clone
    copy rexx/#? df0:rexx clone


#################### Functions and Last Ditch Efforts ################################
# Last ditch default rule so your makes still work.
.DEFAULT :
    @echo "Don't know how to make $@, so I'm touching it."
    touch $@


# Function to perform revision on a given program.
define revise
    revit $@ pn $@ com "by J.W. Lockhart"
    $(AS) $@_rev.asm $(ASFLAGS) $@_rev.o
endef

#################################### END ############################################
