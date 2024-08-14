
SUBSYSTEMS    = a
AFILES        = startup.asm
PRE_COMPILE   = endcode.lib
PUBLIC_IFILES = lowlevel.i
PUBLIC_HFILES = lowlevel.h
CLEANFILES    = lowlevel.map #?.disk
OTHERFILES    = makefile_master makefile lowlevel_lib.sfd \
	        lowlevelbase.i test.c endcode.asm macros.i

OTHERFILES_NORCS = lowlevel_rev.i lowlevel_rev.h lowlevel_rev.rev\
		dependencies

LINKER_OPTS  = SMALLCODE SMALLDATA MAP $*.map XOHSFL PLAIN
LINKER_LIBS  = LIB:amiga.lib endcode.lib

PRE_INTERNAL  = rev lowlevel.ld.disk
POST_INTERNAL =

all:

.include makefile_master

endcode.lib: endcode.asm
	hx68 from endcode.asm TO endcode.lib INCDIR $(INCLUDEDIR) $(ASMOPTS) QUIET
