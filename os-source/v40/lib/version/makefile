######################################################################
#
# Makefile to generate an embedded $VER version string for amiga.lib
# rom.lib, and small.lib.
#

MAKEMETA=	../../tools/makemeta

SUBSYSNAME=	version

AFILES=		amiga_lib.asm rom_lib.asm small_lib.asm

OFILES=		amiga_lib.obj rom_lib.obj small_lib.obj

# Make the pieces.  Blow away _rev.i to force a bump next time around.
all quick:	$(OFILES) amiga.lib rom.lib small.lib
		rm amiga_lib_rev.i rom_lib_rev.i small_lib_rev.i

amiga_lib.obj:	amiga_lib_rev.i

rom_lib.obj:	rom_lib_rev.i

small_lib.obj:	small_lib_rev.i

# This is a bit funky, but the goal is to always keep the
# version numbers of amiga.lib, small.lib and rom.lib in lock-step.

newversion: amiga_lib_rev.rev
# First, get rid of the old, to be sure
		@rm -f amiga_lib_rev.i
		@rm -f rom_lib_rev.i
		@rm -f small_lib_rev.i
# Now, base the other two version files on amiga_lib_rev.rev
		@cp amiga_lib_rev.rev rom_lib_rev.rev
		@cp amiga_lib_rev.rev small_lib_rev.rev
		@${BUMPREV} ${VERNUM} amiga_lib_rev
		@${BUMPREV} ${VERNUM} rom_lib_rev
		@${BUMPREV} ${VERNUM} small_lib_rev
		

# These funny empty rules depend on the above "group version" rule.  They
# contain a single @ sign so the default rule from Makemeta or whereever
# doesn't take effect.

amiga_lib_rev.i: newversion
	@

rom_lib_rev.i: newversion
	@
	
small_lib_rev.i: newversion
	@

amiga.lib:	FRC
		cp amiga_lib.obj amiga.lib

rom.lib:	FRC
		cp rom_lib.obj rom.lib

small.lib:	FRC
		cp small_lib.obj small.lib

.INCLUDE=${MAKEMETA}
