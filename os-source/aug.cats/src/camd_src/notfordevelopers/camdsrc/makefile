########################################################################
# makefile for (C)ommodore (A)miga (M)ultimedia (D)river (camd.library)#
# (c) 1990 by Commodore Business Machines Inc.                         #
########################################################################
# --------------------- required environment
#
# commands available on path:
#   autodoc - CBM autodoc extracter
#   bumprev - CBM revision bumper
#   HX68    - Adapt Assembler (compatible with C.A.P.E. 2.50)
#   lc, lc1, lc2, blink
#           - SAS/C 5.10b
#   join    - Arp or C= 2.0 Join command (1.3 Join command didn't support
#             enough files)
#   Manx make or SAS lmk
#
# assigns
#   lib:     - pointer to SAS/C and C= libraries
#   ainc:    - pointer to C= assembly include files
#
# environment variables
#   SASCOPTS - SAS/C options as defined below

# --------------------- misc definitions

# library version
VER=37

# directory aliases
INCLUDE=/ninclude.both/
GLUE=/glue/
FD=$(INCLUDE)fd/


# --------------------- makefile entry points

all:     lib
lib:     camd.library
pragmas: $(INCLUDE)pragmas/camd_pragmas.h


# --------------------- symbols and rules

# use SYM=ADDSYM for debugging
#     SYM=NODEBUG to dispose of all symbol hunks in the output .library
SYM=ADDSYM

# SAS/C 5.10b
# small code/data, 32 bit ints
# SASCOPTS=-cfsqu -v -O -qt: -i<include path>

XSASCOPTS=-j*e -cfsqu -v -O -qt:
CFLAGS=-i/ninclude.both -q5e $(XSASCOPTS)

# XSASCOPTS are broken out separately to permit an alias like this:
#    alias makelt make XSASCOPTS=
# to turn off warning->error promotion

# -cf  - Force function prototype checking
# -cq  - Permit nested structures to be assigned from one to another
# -cs  - Optimize multiple copies of string constants
# -cu  - Unsigned chars
# -v   - Disable stack checking
# -O   - use global optimizer
# -q5e - Quit after 5 errors
# -j*e - promote all warnings to errors

.c.o:
        lc $(CFLAGS) -Hcamdlib.hpre $*.c


# ADAPT assembler

AFLAGS=-cqry -i /ninclude.both+AInc:
AS=hx68

# -cq - optimizations
# -cr - pc-relative
# -cy - suppress copyright

.asm.o:
        $(AS) -a $*.asm $(AFLAGS) -o$*.o

#        casm -a $*.asm -o $@ -H camdlib.ipre $(AFLAGS) -l $*.lst
# @@@ using a pre-assembled include file doesn't seem to work right
#       . push_Reg## values start at a strange place instead of 0
#       . doesn't do branch optimizations correctly


# --------------------- include file dependencies

# use camdlib.hpre (precompiled include file) to speed up compilation a bit.
# dependency on camd_pragmas.h forces auto fd->pragmas update on .fd file changes

#camdlib.h: $(INCLUDE)midi/camd.h $(INCLUDE)midi/camdbase.h $(INCLUDE)clib/camd_protos.h $(INCLUDE)pragmas/camd_pragmas.h
#        @wait
#        setdate $@

camdlib.hpre: camdpre.c camdlib.h $(INCLUDE)midi/camd.h $(INCLUDE)midi/camdbase.h $(INCLUDE)clib/camd_protos.h $(INCLUDE)pragmas/camd_pragmas.h
        lc $(CFLAGS) -ph -o$@ camdpre.c

camdlib.i: $(INCLUDE)midi/camd.i $(INCLUDE)midi/camdbase.i
        wait
        setdate $@

#camdlib.ipre: camdpre.asm camdlib.i /include/midi/camd.i /include/midi/camdbase.i
#        casm -a camdpre.asm -o $@ $(AFLAGS)

# C modules depend .hpre instead of .h to keep dependencies a little simpler

device.o:  camdlib.hpre
libc.o:    camdlib.hpre
link.o:    camdlib.hpre
lock.o:    camdlib.hpre
misc.o:    camdlib.hpre
node.o:    camdlib.hpre
serialc.o: camdlib.hpre
unitc.o:   camdlib.hpre

head.o:    camdlib.i camd_rev.i
liba.o:    camdlib.i
msg.o:     camdlib.i
parse.o:   camdlib.i
port.o:    camdlib.i
seriala.o: camdlib.i
unita.o:   camdlib.i


# --------------------- special compilation / assembly


# --------------------- object module grouping

# !!! this needs to be updated
# Modules
#   head.asm    - Head module.  Contains resident tag, date stamps, etc
#
#   liba.asm    - Main asm module.  lib vectors and std library funcs:
#                 open, close, expunge, etc.
#   libc.c      - Main C module.  C section initialization.
#   csup.asm    - compiler specific C environment support.
#   client.c    - camd application management functions
#   device.c    - Device driver support.
#   link.c      - MidiLink allocator and various Cluster related functions.
#   lock.c      - List locking functions.
#   msg.asm     - MidiMsg processing.
#   misc.c      - Misc internal functions.
#   node.c      - MidiNode allocator and various MI related functions.
#   notify.c    - External notification of CAMD internal state changes
#   parse.asm   - MIDI parsers.
#   seriala.asm - Assembly section of internal serial I/O handler.
#   serialc.c   - C section of internal serial I/O handler.
#   unita.asm   - Assembly section of Unit handling code.
#   unitc.c     - C section of Unit handling code.
#
#   tail.asm    - Tail module.

CAMD_O=liba.o libc.o csup.o device.o link.o lock.o misc.o msg.o node.o parse.o\
  serialc.o seriala.o unitc.o unita.o notify.o\
  prtmsg.o


# --------------------- revision tracking

# _rev.i file gets updated whenever any other module changes.
# !!! revision bumping made manual

#camd_rev.i: $(CAMD_O) $(GLUE)camd.lib
bump:
        wait
        bumprev $(VER) camd_rev


# --------------------- linkage
#        echo >$@ "FROM $(CAMD_O)"

camd.lnk: $(CAMD_O)
        wait
        execute make.script "FROM $(CAMD_O)"

camd.library: head.o camd.lnk tail.o $(GLUE)camd.lib
        blink TO $@ \
            FROM head.o WITH camd.lnk LIB $(GLUE)camd.lib lib:amiga.lib lib:lc.lib lib:debug.lib tail.o \
            SC BATCH $(SYM) \
            MAP $*.map fhsx plain pwidth 16 swidth 24
        protect $@ -e

full: camd.library
	copy camd.library libs:

# library order:  amiga.lib before lc.lib to get amiga.lib version of sprintf()

#CAMD_OS=liba.o libc.o csup.o device.o link.o lock.o misc.o msg.o node.o parse.o\
#        serialc.o seriala.o timerc.o timera.o unitc.o unita.o

#camd.os: $(CAMD_OS)
#        @wait
#        join $(CAMD_OS) as $@

#camd.library: head.o camd.os tail.o
#        blink FROM head.o camd.os TO $@ LIB $(GLUE)camd.lib lib:amiga.lib lib:lc.lib lib:debug.lib tail.o \
#              sc batch $(SYM) MAP $*.map fhsx plain pwidth 16 swidth 24
#        @protect $@ -e


# --------------------- pragmas

$(INCLUDE)pragmas/camd_pragmas.h: $(FD)camd_lib.fd
        fd2pragma $(FD)camd_lib.fd $(INCLUDE)pragmas/camd_pragmas.h


# --------------------- autodoc

#doc:
#       autodoc camd.doc from *.(c|asm) title camd.library
#       autodoc mididev.doc from mididev.dsrc banner "Camd Library Functions"

doc:
	autodoc >camd.autodoc -I -t8 -l80 -C unitc.c unita.asm misc.c serialc.c seriala.asm link.c msg.asm libc.c liba.asm csup.asm node.c notify.o device.c parse.asm lock.c notify.c
