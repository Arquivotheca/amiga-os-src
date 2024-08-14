# Makefile for Services Manager and services.library
# (this makefile is set up for bmake, BTW)

LIB                 = services
MGR                 = manager

VERSION             = 37

REMOTE              = APPN:src/services

LD_LIB              = $(LIB).ld
LD_LIB_STRIP        = $(LIB).ld.strip

LD_MGR              = $(MGR).ld
LD_MGR_STRIP        = $(MGR).ld.strip

C_COMPILER          = LC
LIB_C_OPTS          = -b0 -. -ms -me -v -rr -O -cfistqmcu -d1
MGR_C_OPTS          = -. -ms -v     -rr -O -cfistqmcu -d1

ASSEMBLER           = hx68 -a
ASSEMBLER_OPTS      = -csy -i include: -o

LINKER              = Blink
LIB_LINKER_OPTS     = ADDSYM SC SD MAP $(LIB).map HLSXF PLAIN SWIDTH 16 NOICONS
LINKER_OPTS_STRIP   = ND

MGR_LINKER_OPTS     = ADDSYM SC SD MAP $(MGR).map HLSXF PLAIN SWIDTH 16

LIB_C_SOURCES       = flushlib.c findservice.c loseservice.c
LIB_ASM_SOURCES     = services_lib.asm stubs.asm endcode.asm

LIB_OBJECTS         = services_lib.o flushlib.o findservice.o loseservice.o stubs.o endcode.o

LIB_LIBS            = LIB:debug.lib LIB:small.lib LIB:envoy.lib

MGR_C_SOURCES       = manager.c

MGR_OBJECTS         = manager.o

MGR_LIBS            = LIB:debug.lib LIB:amiga.lib LIB:envoy.lib

all:    $(LD_LIB_STRIP) $(LD_MGR_STRIP)

##############################################################################
# Build the stripped load files...
#
$(LD_LIB_STRIP):    $(LD_LIB)
                    $(LINKER) FROM $(LD_LIB) TO $(LD_LIB_STRIP) $(LINKER_OPTS_STRIP)
                    copy $(LD_LIB_STRIP) libs:services.library

$(LD_MGR_STRIP):    $(LD_MGR)
                    $(LINKER) FROM $(LD_MGR) TO $(LD_MGR_STRIP) $(LINKER_OPTS_STRIP)
                    copy $(LD_MGR_STRIP) Manager

##############################################################################
# Build the load files...
#
$(LD_LIB):  $(LIB_OBJECTS) $(LIB_LIBS)
            $(LINKER) $(LIB_OBJECTS) TO $(LD_LIB) LIB $(LIB_LIBS) $(LIB_LINKER_OPTS)

$(LD_MGR):  $(MGR_OBJECTS) $(MGR_LIBS)
            $(LINKER) $(MGR_OBJECTS) TO $(LD_MGR) LIB $(MGR_LIBS) $(MGR_LINKER_OPTS)

##############################################################################
# Default rules...
#
%.o: %.c
    $(C_COMPILER) $(LIB_C_OPTS) -o$@ $<

%.o: %.asm
    $(ASSEMBLER) $< $(ASSEMBLER_OPTS) $@

%.asm:

%.c:

%.rev:

%.lib:

%.h:

%.i:

##############################################################################
# All dependencies...
#
services_lib.o  : services_lib.asm services_rev.rev services.i servicesbase.i
servicesbase.o  : servicesbase.c servicesbase.h services.h include:envoy/nipc.h
findservice.o   : findservice.c  servicesbase.h services.h servicesinternal.h include:envoy/nipc.h
startservice.o  : startservice.c servicesbase.h services.h servicesinternal.h include:envoy/nipc.h
stopservice.o   : stopservice.c  servicesbase.h services.h servicesinternal.h include:envoy/nipc.h
flushlib.o      : flushlib.c     servicesbase.h services.h servicesinternal.h include:envoy/nipc.h
loseservice.o   : loseservice.c  servicesbase.h services.h servicesinternal.h include:envoy/nipc.h
stubs.o         : stubs.asm servicesbase.i

manager.o       : manager.c servicesbase.h services.h servicesinternal.h manager_rev.rev include:envoy/nipc.h
                lc -b0 -. -v -ms -rr -O -cfistqmcu -d1 -omanager.o manager.c

#############################################################################
# Bump services.library revision
#
libversion:
    -co -l $(LIB)_rev.rev
    -co -l $(LIB)_rev.i
    -co -l $(LIB)_rev.h
    -BumpRev $(VERSION) $(LIB)_rev
    -ci -u -m"" -r$(VERSION) $(LIB)_rev.rev
    -ci -u -m"" -r$(VERSION) $(LIB)_rev.i
    -ci -u -m"" -r$(VERSION) $(LIB)_rev.h

#############################################################################
# Bump Services Manager revision
#
mgrversion:
    -co -l $(MGR)_rev.rev
    -co -l $(MGR)_rev.i
    -co -l $(MGR)_rev.h
    -BumpRev $(VERSION) $(MGR)_rev
    -ci -u -m"" -r$(VERSION) $(MGR)_rev.rev
    -ci -u -m"" -r$(VERSION) $(MGR)_rev.i
    -ci -u -m"" -r$(VERSION) $(MGR)_rev.h

##############################################################################
# Delete all object files
#
clean:
    -Delete $(LIB_OBJECTS) $(MGR_OBJECTS)
    -Delete $(LD_LIB) $(LD_LIB_STRIP) services.map
    -Delete $(LD_MGR) $(LD_MGR_STRIP) manager.map
    -Delete FD LVO HDR PROTOS PRAGMAS RAM ROM LIB all quiet force

##############################################################################
# Build autodoc
#
services.doc:   autodocs.asm
                -Autodoc >services.doc autodocs.asm

sfd:
    sfd services_lib.sfd
    copy RAM/services/#?.asm LIB
    copy LVO LIB
    copy PROTOS include:clib
    copy PRAGMAS include:pragmas

##############################################################################
# Release the current build to the server...
#
internal:
    -Delete $(REMOTE)/((#?.(c|h|asm|i|strip|ld|rev|doc|library))|makefile)
    -protect ((#?.(c|h|asm|i|ld|strip|rev|doc))|makefile) +r quiet
    -copy #?.(c|h|i|asm|sfd|strip|ld|doc|library) #?_rev.#? $(REMOTE) NOPRO

release:
    -copy services.(i|h) appn:include/envoy NOPRO
    -copy services.doc appn:doc NOPRO
