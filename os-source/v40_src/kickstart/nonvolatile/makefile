
SUBSYSTEMS    = NVRAM DISK
AFILES        = start.asm main.asm
CFILES        = oldmain.c
PRE_COMPILE   = endcode.lib
PUBLIC_IFILES = nonvolatile.i
PUBLIC_HFILES = nonvolatile.h
CLEANFILES    = nonvolatile.map
OTHERFILES    = makefile_master makefile nonvolatile_lib.sfd \
	        nonvolatilebase.i nonvolatilebase.h test.c endcode.asm

OTHERFILES_NORCS = nonvolatile_rev.i nonvolatile_rev.h nonvolatile_rev.rev\
		dependencies

LINKER_OPTS  = SMALLCODE SMALLDATA MAP $*.map XOHSFL PLAIN
LINKER_LIBS  = LIB:amiga.lib endcode.lib

PRE_INTERNAL  = rev
POST_INTERNAL =

.include makefile_master

endcode.lib: endcode.asm
	hx68 from endcode.asm TO endcode.lib INCDIR $(INCLUDEDIR) $(ASMOPTS) QUIET
