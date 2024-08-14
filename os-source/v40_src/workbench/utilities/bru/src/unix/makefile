#
#  FILE
#
#	Makefile    makefile for bru source directory
#
#  SCCS
#
#	@(#)Makefile	12.8	11 Feb 1991
#
#  SYNOPSIS
#
#	cd bru/src/unix; make [targets]
#
#  DESCRIPTION
#
#	This is the makefile which controls compilation of the source
#	files to produce an executable bru utility.   It is normally
#	invoked by a recursive make in the bru root directory via
#	the bru.mk makefile.
#
#	Useful things to make are:
#
#		bru	  =>	Remake the bru executable in local
#				directory.
#
#		lint  =>	Do a complete lint on the sources.
#
#		install   =>	Install the new bru executable.
#
#		clean     =>	Clean up source directory by removing
#				objects, *.BAK files, etc.
#
#		superclean =>	Remove everything except SCCS sources.
#
#

#
#	Define defaults for various macros.  These can be overridden
#	by command line arguments to "make" of the form <macro>=<definition>.
#	If you are using autoconfiguration, you don't need to worry about
#	most of these, they will be automatically overridden.
#
#	The meanings of each macro are:
#
#		BIN	=>	directory in which to install executable
#
#		DBUG	=>	left null if "dbug" package is available
#				and wanted, set to "-DNO_DBUG" if unavailable
#				or to "-DDBUG_OFF" if available but not
#				wanted.
#
#		LIBDBUG	=>	set to "-ldbug" to load "dbug" runtime
#				support library.
#
#		MAKE	=>	The make program to use.  On usg type systems,
#				with the new augmented make, this is a builtin
#				macro.  Must be defined here for the sake of
#				bsd flavor systems.
#
#		ROOT	=>	Normally left null for native compiles.
#				Set to root directory of your compilation
#				tree for cross compiles.
#

BIN =		$(ROOT)/bin
DBUG =
LIBDBUG =
MAKE =		/bin/make
ROOT =

COMDIR =	../common
UNIXDIR =	../unix
INCLUDE =	-I$(COMDIR) -I$(UNIXDIR)
MODE =		4711
GRP =		bin
OWN =		root
OFLAG = 	-O
GFLAGS =	-s
PROF =
OTHERCFLAGS =
OTHERLDFLAGS =
SHELL =		/bin/sh

#
#	Macros which are composites of other macros.
#

CDEFINES =	$(DEFS) $(DBUG) $(TESTONLY)
CFLAGS =	$(OFLAG) $(CDEFINES) $(INCLUDE) $(PROF) $(OTHERCFLAGS)
LDFLAGS =	$(OTHERLDFLAGS)
LINTLIBS =	$(LIBS) $(LIBDBUG)
LINTDEFS =	$(CDEFINES) $(INCLUDE) $(OTHERCFLAGS)

#
#	List of object files.
#
#

CFILES =	$(COMDIR)/access.c $(COMDIR)/blocks.c $(COMDIR)/bru.c \
		$(COMDIR)/brutab.c $(COMDIR)/chksum.c $(COMDIR)/compress.c \
		$(COMDIR)/create.c $(COMDIR)/date.c $(UNIXDIR)/dblib.c \
		$(COMDIR)/devices.c $(COMDIR)/diff.c $(COMDIR)/done.c \
		$(COMDIR)/estimate.c \
		$(COMDIR)/execute.c $(COMDIR)/extract.c $(UNIXDIR)/filestat.c \
		$(COMDIR)/filter.c $(COMDIR)/fmode.c $(COMDIR)/getopt.c \
		$(COMDIR)/globals.c $(COMDIR)/group.c $(COMDIR)/headers.c \
		$(COMDIR)/hex.c $(COMDIR)/info.c $(COMDIR)/init.c \
		$(COMDIR)/inspect.c $(COMDIR)/links.c $(COMDIR)/passwd.c \
		$(COMDIR)/readinfo.c $(COMDIR)/scan.c $(COMDIR)/signals.c \
		$(UNIXDIR)/stubs.c \
		$(COMDIR)/symlinks.c $(COMDIR)/sys.c $(COMDIR)/sys2.c \
		$(COMDIR)/sys3.c $(COMDIR)/sys4.c $(COMDIR)/table.c \
		$(COMDIR)/trees.c $(COMDIR)/tty.c $(COMDIR)/usage.c \
		$(COMDIR)/usermsg.c $(COMDIR)/utils.c $(COMDIR)/wild.c

HFILES =	$(COMDIR)/blocks.h $(COMDIR)/bruinfo.h $(COMDIR)/config.h \
		$(COMDIR)/dbug.h $(COMDIR)/devices.h $(COMDIR)/dir.h \
		$(COMDIR)/exeinfo.h $(COMDIR)/finfo.h \
		$(COMDIR)/flags.h $(COMDIR)/globals.h $(COMDIR)/macros.h \
		$(COMDIR)/manifest.h $(COMDIR)/rmt.h $(COMDIR)/trees.h \
		$(COMDIR)/typedefs.h $(COMDIR)/usermsg.h $(COMDIR)/utsname.h \
		$(COMDIR)/vargs.h $(UNIXDIR)/autoconfig.h $(UNIXDIR)/deftab.h

OFILES =	access.o blocks.o bru.o brutab.o chksum.o compress.o create.o \
		date.o dblib.o devices.o diff.o done.o estimate.o \
		execute.o extract.o filestat.o filter.o fmode.o globals.o \
		group.o headers.o hex.o info.o init.o inspect.o links.o \
		passwd.o readinfo.o scan.o signals.o stubs.o symlinks.o \
		sys.o sys2.o sys3.o sys4.o table.o trees.o tty.o usage.o \
		usermsg.o utils.o wild.o


#	The makefile is set up such that if invoked without an
#	explicit target to make, it attempts to do autoconfiguration
#	via the shell script "make.sh", which then reinvokes make
#	with the appropriate explicit target and defines.

default:
	$(SHELL) ./make.sh

#
#	Make bru using defines set up by make.sh
#

bru :		$(OFILES)
		$(CC) $(LDFLAGS) -o $@ $(OFILES) $(LIBS)

#
#	Make brutalk using defines set up by make.sh
#

brutalk :	brutalk.o getopt.o
		$(CC) $(LDFLAGS) -o $@ brutalk.o getopt.o $(LIBS)

#
#	Make shmtest using defines set up by make.sh
#

shmtest :	shmtest.c autoconfig.h
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ shmtest.c $(LIBS)

#
#	The object files depend on the header files.
#
#	In theory, only the dependency line is needed.  In practice,
#	most Unix makes do not know how to apply the built in rules to
#	files in another directory, so we must have explicit action lines.
#

access.o:	$(COMDIR)/access.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/access.c

blocks.o:	$(COMDIR)/blocks.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/blocks.c

bru.o:		$(COMDIR)/bru.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/bru.c

brutab.o:	$(COMDIR)/brutab.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/brutab.c

brutalk.o:	$(UNIXDIR)/brutalk.c $(HFILES)
		$(CC) -c $(CFLAGS) $(UNIXDIR)/brutalk.c

chksum.o:	$(COMDIR)/chksum.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/chksum.c

compress.o:	$(COMDIR)/compress.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/compress.c

create.o:	$(COMDIR)/create.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/create.c

date.o:		$(COMDIR)/date.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/date.c

dblib.o:	$(UNIXDIR)/dblib.c $(HFILES)
		$(CC) -c $(CFLAGS) $(UNIXDIR)/dblib.c

devices.o:	$(COMDIR)/devices.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/devices.c

diff.o:		$(COMDIR)/diff.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/diff.c

done.o:		$(COMDIR)/done.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/done.c

estimate.o:	$(COMDIR)/estimate.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/estimate.c

execute.o:	$(COMDIR)/execute.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/execute.c

extract.o:	$(COMDIR)/extract.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/extract.c

filestat.o:	$(UNIXDIR)/filestat.c $(HFILES)
		$(CC) -c $(CFLAGS) $(UNIXDIR)/filestat.c

filter.o:	$(COMDIR)/filter.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/filter.c

fmode.o:	$(COMDIR)/fmode.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/fmode.c

getopt.o:	$(COMDIR)/getopt.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/getopt.c

globals.o:	$(COMDIR)/globals.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/globals.c

group.o:	$(COMDIR)/group.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/group.c

headers.o:	$(COMDIR)/headers.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/headers.c

hex.o:		$(COMDIR)/hex.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/hex.c

info.o:		$(COMDIR)/info.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/info.c

init.o:		$(COMDIR)/init.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/init.c

inspect.o:	$(COMDIR)/inspect.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/inspect.c

links.o:	$(COMDIR)/links.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/links.c

passwd.o:	$(COMDIR)/passwd.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/passwd.c

readinfo.o:	$(COMDIR)/readinfo.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/readinfo.c

scan.o:		$(COMDIR)/scan.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/scan.c

signals.o:	$(COMDIR)/signals.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/signals.c

stubs.o:	$(UNIXDIR)/stubs.c $(HFILES)
		$(CC) -c $(CFLAGS) $(UNIXDIR)/stubs.c

symlinks.o:	$(COMDIR)/symlinks.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/symlinks.c

sys.o:		$(COMDIR)/sys.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/sys.c

sys2.o:		$(COMDIR)/sys2.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/sys2.c

sys3.o:		$(COMDIR)/sys3.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/sys3.c

sys4.o:		$(COMDIR)/sys4.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/sys4.c

table.o:	$(COMDIR)/table.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/table.c

trees.o:	$(COMDIR)/trees.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/trees.c

tty.o:		$(COMDIR)/tty.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/tty.c

usage.o:	$(COMDIR)/usage.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/usage.c

usermsg.o:	$(COMDIR)/usermsg.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/usermsg.c

utils.o:	$(COMDIR)/utils.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/utils.c

wild.o:		$(COMDIR)/wild.c $(HFILES)
		$(CC) -c $(CFLAGS) $(COMDIR)/wild.c

#
#	The autoconfig.h file is normally built by the make.sh script.
#	If we change make.sh, then this dependency makes sure that
#	we remake autoconfig.h.

$(UNIXDIR)/autoconfig.h:	./config
		$(SHELL) ./config

#
#	Lint the sources.
#	(Some systems forget to predefine 'lint', so do it anyway)
#

lint :
		lint -Dlint $(LINTDEFS) $(LINTFLAGS) $(CFILES) $(LINTLIBS) >lint.out

#
#	Update globally available version with the local test
#	version.  You must be superuser to do this.
#	Note that this also assumes that there is a local test
#	version.  "install" could be made dependent on "bru" but
#	then recursive makes from bru.mk would need to pass
#	along all the other appropriate defines in case "bru" had
#	to be remade.  This is primarily a matter of taste.
#

install :	$(BIN)/bru 

$(BIN)/bru :	bru
		if test -f $@ ;then mv $@ $(BIN)/OLDbru ;fi
		cp bru $@
		chown $(OWN) $@
		chgrp $(GRP) $@
		chmod $(MODE) $@

#
#	Clean up by removing various things, such as the object files.
#

clean :
		rm -f nohup.out *.o *.BAK *.old

clobber :
		rm -f nohup.out *.o *.BAK *.old bru

#
#	Make the directory superclean by removing everything
#	except SCCS sources.  Note that the remove is done
#	interactively to prevent any catastrophic mistakes.
#

superclean :
		rm -i ?[!.]*
