######################################################################
MAKEMETA=	../../tools/makemeta

SUBDIRS=	RAM ROM LVO

all:		lock asm.timestamp ${SUBDIRS} amiga.lib rom.lib small.lib unlock

quick:		lock ${SUBDIRS} amiga.lib rom.lib unlock

sfdheaders:	lock hdr.timestamp unlock

clean::		lock unlock
		-rm FD/* HDR/* PRAGMAS/* PROTOS/* asm.timestamp hdr.timestamp

# The list of modules is maintained by hand.  If you add a module,
# you must add it to V:lib/sfd/Makefile, V:lib/sfd/(RAM|ROM|LVO)/Makefile,
# and mkdir V:lib/sfd/(RAM|ROM)/subsystem!

XFDFILES=	amigaguide_lib.sfd \
		asl_lib.sfd \
		battclock_lib.sfd \
		battmem_lib.sfd \
		bullet_lib.sfd \
		cardres_lib.sfd \
		cia_lib.sfd \
		colorwheel_lib.sfd \
		commodities_lib.sfd \
		console_lib.sfd \
		datatypes_lib.sfd \
		disk_lib.sfd \
		diskfont_lib.sfd \
		dos_lib.sfd \
		exec_lib.sfd \
		expansion_lib.sfd \
		gadtools_lib.sfd \
		graphics_lib.sfd \
		icon_lib.sfd \
		iffparse_lib.sfd \
		input_lib.sfd \
		intuition_lib.sfd \
		keymap_lib.sfd \
		layers_lib.sfd \
		locale_lib.sfd \
		lowlevel_lib.sfd \
		mathffp_lib.sfd \
		mathieeedoubbas_lib.sfd \
		mathieeedoubtrans_lib.sfd \
		mathieeesingbas_lib.sfd \
		mathieeesingtrans_lib.sfd \
		mathtrans_lib.sfd \
		misc_lib.sfd \
		nonvolatile_lib.sfd \
		potgo_lib.sfd \
		ramdrive_lib.sfd \
		realtime_lib.sfd \
		rexxsyslib_lib.sfd \
		timer_lib.sfd \
		translator_lib.sfd \
		utility_lib.sfd \
		wb_lib.sfd


# Kevin put in lock/unlock code to prevent two people from simultaneously
# making the SFD stuff.  I've removed this due to several problems:
# - The makefiles don't handle file ownership issues correctly, so
#   it's not currently possible to have more than one person building
#   in this directory anyway.
# - When a module owner changes an SFD file, a syntax error in the SFD
#   file isn't that unlikely, and we don't find out until we get here.
#   Then you have to manually "make unlock" to retry.

lock:
#		whoami >sfd.lock
#		chmod 444 sfd.lock
#		test "`whoami`" = "`cat sfd.lock`"

unlock:
#		rm -f sfd.lock

asm.timestamp:	${XFDFILES}
		CleanLib $?
		../../tools/bin/sfd $?
		touch asm.timestamp
		mv FD/* ${VERDIR}/fd
		mv HDR/* ${SYSINCLNS}/internal
		if test -r PRAGMAS/exec_pragmas.h; then ( \
		    sed -e "s/libcall SysBase/syscall/" PRAGMAS/exec_pragmas.h \
		      >${VERDIR}/inc.lattice/pragmas/exec_old_pragmas.h \
		); fi
		mv PRAGMAS/* ${VERDIR}/inc.lattice/pragmas
		mv PROTOS/* ${SYSINCLNS}/clib
		touch hdr.timestamp

hdr.timestamp:	${XFDFILES}
		../../tools/bin/sfd -h $?
		mv FD/* ${VERDIR}/fd
		mv HDR/* ${SYSINCLNS}/internal
		if test -r PRAGMAS/exec_pragmas.h; then ( \
		    sed -e "s/libcall SysBase/syscall/" PRAGMAS/exec_pragmas.h \
		      >${VERDIR}/inc.lattice/pragmas/exec_old_pragmas.h \
		); fi
		mv PRAGMAS/* ${VERDIR}/inc.lattice/pragmas
		mv PROTOS/* ${SYSINCLNS}/clib
		touch hdr.timestamp

amiga.lib:	FRC
		cat LVO/*.obj RAM/amiga.lib >amiga.lib
		rm RAM/amiga.lib

rom.lib:	FRC
		cat LVO/*.obj ROM/rom.lib >rom.lib
		rm ROM/rom.lib

small.lib:	FRC
		cat LVO/*.obj >small.lib

.INCLUDE=${MAKEMETA}
