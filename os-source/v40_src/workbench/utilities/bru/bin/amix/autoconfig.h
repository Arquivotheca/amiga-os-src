/*
 *	This autoconfiguration file was built automatically by make.sh.
 *	Any handmade changes will get overwritten the next time
 *	autoconfiguration is done, thus it is best to change make.sh as
 *	necessary.
 */

/*
 *	Define whether or not the C compiler under xenix handles
 *	the "far" keyword without error.  Some compilers won't
 *	even compile their own header files!  In this case, we
 *	just define the "far" away and hope for the best.  Bru
 *	does not use any "far" keywords.  This is only applicable
 *	to xenix.  If the line "#define far" does not appear below
 *	then you are not running xenix, or your compiler is ok.
 *	(Ditto comments above for "near" keyword.)
 */

/* #define far */
/* #define near */

/*
 *	Define whether or not the C compiler fully understands the
 *	"void" type.  If not, define BROKEN_VOID to 1.  Otherwise,
 *	leave undefined or define to 0.
 */

/* #define BROKEN_VOID 0 */

/*
 *	Define whether or not native characters are signed or unsigned.
 *	If signed, set CHARS_ARE_SIGNED to 1.
 */

#ifndef CHARS_ARE_SIGNED
#define CHARS_ARE_SIGNED 0
#endif

/*
 *	Define whether or not the explicit type "signed char" is 
 *	acceptable to the compiler.
 */

#ifndef SIGNED_CHARS_OK
#define SIGNED_CHARS_OK 1
#endif

/*
 *	Define whether or not the explicit type "unsigned char" is 
 *	acceptable to the compiler.
 */

#ifndef UNSIGNED_CHARS_OK
#define UNSIGNED_CHARS_OK 1
#endif

/*
 *	Define whether or not statvfs() can be found in libc.a along with
 *	the include file <statvfs.h>.  This call can be used to avoid
 *	non-filesystem thingies like /proc or /dev/fd under SVR4.
 */

#ifndef HAVE_STATVFS
#define HAVE_STATVFS 1
#endif

/*
 *	Define whether or not uname() can be found in libc.a.  If not, then
 *	we need to use our own private copy.  This will usually be missing
 *	on some older BSD derived systems.  Also define whether or not the
 *	<sys/utsname.h> include file is available, and if so, does the
 *	utsname structure has a "machine" member (some do not).
 */

#ifndef HAVE_UNAME
#define HAVE_UNAME 1
#endif

#ifndef HAVE_UTSNAME
#define HAVE_UTSNAME 1
#endif

#ifndef HAVE_UTSNAME_MACHINE
#define HAVE_UTSNAME_MACHINE 1
#endif

/*
 *	Define whether or not you have BSD style waits.
 */

#ifndef HAVE_UNIONWAIT
#define HAVE_UNIONWAIT 0
#endif

/*
 *	Define whether or not tzset() can be found in libc.a.  If not, then
 *	we are probably on a BSD type system and can use gettimeofday
 *	instead.
 */

#ifndef HAVE_TZSET
#define HAVE_TZSET 1
#endif

/*
 *	Define whether or not gettimeofday() can be found in libc.a.  If 
 *	we don't have tzset and timezone, we can use this instead.  Usually
 *	found on BSD flavored systems.
 */

#ifndef HAVE_GETTIMEOFDAY
#define HAVE_GETTIMEOFDAY 1
#endif

/*
 *	Define whether or not getopt() can be found in libc.a.  If not, then
 *	we need to use our own private copy.  This will usually be missing
 *	on some older BSD derived systems.
 */

#ifndef HAVE_GETOPT
#define HAVE_GETOPT 1
#endif

/*
 *	Define whether or not strtok() can be found in libc.a.  If not, then
 *	we need to use our own private copy.  This will usually be missing
 *	on some older BSD derived systems.
 */

#ifndef HAVE_STRTOK
#define HAVE_STRTOK 1
#endif

/*
 *	Define whether or not strtol() can be found in libc.a.  If not, then
 *	we need to use our own private copy.  This will usually be missing
 *	on some older BSD derived systems.
 */

#ifndef HAVE_STRTOL
#define HAVE_STRTOL 1
#endif

/*
 *	Define whether or not mkdir() can be found in libc.a.  If not, then
 *	we need to build directory nodes the hard way.  Most BSD derived 
 *	systems, or USG systems extended to use NFS, will have mkdir().
 */

#ifndef HAVE_MKDIR
#define HAVE_MKDIR 1
#endif

/*
 *	Define whether or not ulimit() can be found in libc.a.
 */

#ifndef HAVE_ULIMIT
#define HAVE_ULIMIT 1
#endif

/*
 *	Define whether or not memset() can be found in libc.a.  If not, then
 *	we do our block memory zero'ing using a portable C loop, which can
 *	be significantly slower.
 */

#ifndef HAVE_MEMSET
#define HAVE_MEMSET 1
#endif

/*
 *	Define whether or not bcopy() can be found in libc.a.
 */

#ifndef HAVE_BCOPY
#define HAVE_BCOPY 0
#endif

/*
 *	Define whether or not strchr() can be found in libc.a.
 */

#ifndef HAVE_STRCHR
#define HAVE_STRCHR 1
#endif

/*
 *	Define whether or not strrchr() can be found in libc.a.
 */

#ifndef HAVE_STRRCHR
#define HAVE_STRRCHR 1
#endif

/*
 *	Define whether or not INDEX() can be found in libc.a.
 */

#ifndef HAVE_INDEX
#define HAVE_INDEX 1
#endif

/*
 *	Define whether or not rindex() can be found in libc.a.
 */

#ifndef HAVE_RINDEX
#define HAVE_RINDEX 0
#endif

/*
 *	Define whether or not memcpy() can be found in libc.a.
 */

#ifndef HAVE_MEMCPY
#define HAVE_MEMCPY 1
#endif

/*
 *	Define whether or not the BSD style directory access routines can
 *	be found in libc.a.  If not, we need to use the internal versions
 *	(set HAVE_SEEKDIR to 0).  It is preferable to use the system
 *	supplied versions if they are available.
 */

#ifndef HAVE_SEEKDIR
#define HAVE_SEEKDIR 1
#endif

/*
 *	Define whether or not the include file <dirent.h> is available.
 *	If so, then HAVE_DIRENT should be set to 1.
 */

#ifndef HAVE_DIRENT
#define HAVE_DIRENT 1
#endif

/*
 *	Bru will attempt to use telldir/closedir/opendir/seekdir
 *	sequences to close directories that it is not actively
 *	using, reducing the number of file descriptors in use.
 *	This helps to prevent recursive directory reading of deeply
 *	nested hierarchies from failing due limits on the number
 *	of simultaneously open file descriptors.  On some systems,
 *	even though the manual pages imply that this sequence
 *	will work, the implementation appears to be broken.  If
 *	so, then define BROKEN_SEEKDIR to 1.
 *
 */

#ifndef BROKEN_SEEKDIR
#define BROKEN_SEEKDIR 0
#endif

/*
 *	Define AUX to 1 if the host is running Apple's A/UX unix
 *	implementation.
 */

#ifndef AUX
#define AUX 0
#endif

/*
 *	Define xenix if it looks like the host is a xenix system and
 *	xenix is not already defined.  Most xenix systems don't define
 *	unix, so one of these MUST be defined.
 */

#ifndef xenix
#define xenix 0
#endif

/*
 *	If any of the source files have been checked out for editing
 *	from SCCS, then define TESTONLY to handle special cases of
 *	non-expansion of sccs keywords.
 */

#ifndef TESTONLY
#define TESTONLY 0
#endif

/*
 *	Define exactly one of SIGTYPEINT or SIGTYPEVOID, to indicate
 *	whether or not signal returns a pointer to a function returning
 *	int, or a pointer to a function return void, respectively.
 */

#if (!SIGTYPEINT && !SIGTYPEVOID)
#define SIGTYPEINT 0
#define SIGTYPEVOID 1
#endif

/*
 *	Define RMT to 1 if we have support for remote shells, on the
 *	assumption that some system out there may have /etc/rmt on it,
 *	thus allowing us to support remote tape drives.
 */

#ifndef RMT
#define RMT 0
#endif

/*
 *	Define HAVE_STDARG to 1 if support is provided for 
 *	the ANSI C <stdarg.h> style variable argument lists.
 *	Otherwise, we will look at HAVE_VARARGS to decide what
 *	to do about variable argument lists.
 */

#ifndef HAVE_STDARG
#define HAVE_STDARG 1
#endif

/*
 *	Define HAVE_VARARGS to 1 if support is provided for 
 *	the varargs macros.  Otherwise, we will fake the varargs
 *	stuff with a local copy, which should work on any system
 *	that doesn't strictly require varargs.
 */

#ifndef HAVE_VARARGS
#define HAVE_VARARGS 1
#endif

/*
 *	Define HAVE_TERMIO to 1 if we have <termio.h> style terminal
 *	handling.
 */

#ifndef HAVE_TERMIO
#define HAVE_TERMIO 1
#endif

/*
 *	Define HAVE_SYMLINKS to 1 if symbolic links are available.
 */

#ifndef HAVE_SYMLINKS
#define HAVE_SYMLINKS 1
#endif

/*
 *	Define HAVE_CONTIGUOUS to 1 if Masscomp style contiguous
 *	files are supported by the host machine.
 */

#ifndef HAVE_CONTIGUOUS
#define HAVE_CONTIGUOUS 0
#endif

/*
 *	Define HAVE_STRANGE_UTIMBUF to 1 your machine does NOT define
 *	a struct utimbuf anywhere (see utime(2)) and requires a utimbuf
 *	that is different than the "classical" definition:
 *
 *		struct utimbuf {
 *			time_t actime;
 *			time_t modtime;
 *		};
 *
 */

#ifndef HAVE_STRANGE_UTIMBUF
#define HAVE_STRANGE_UTIMBUF 0
#endif

/*
 *	Define HAVE_FIFOS to 1 if fifos are available.
 */

#ifndef HAVE_FIFOS
#define HAVE_FIFOS 1
#endif

/*
 *	Define HAVE_SHM to 1 if System V style shared memory is available.
 *	If it is, we can use use double buffering to greatly increase
 *	throughput in some situations (actual improvement is hardware 
 *	dependent).
 */

#ifndef HAVE_SHM
#define HAVE_SHM 1
#endif

/*
 *	Define FAST_CHKSUM to 1 if you are willing to support a private
 *	machine dependant checksum routine (usually written in assembler)
 *	that replaces the simple checksum loop in the function "chksum".
 *	Your routine must be named sumblock, and takes two arguments,
 *	a pointer to the first byte and a byte count.  Be careful to
 *	test your custom version of bru archive read/write compatibility
 *	with the version that uses the C code.  If you know enough to
 *	write this, you are assumed to know enough to figure out how
 *	to change the makefile appropriately.
 */

#ifndef FAST_CHKSUM
#define FAST_CHKSUM 0
#endif

/*
 *	Define CONFIG_DATE as a string of the form given by
 *	the unix "date" command with no arguments".  This is the
 *	date that autoconfig was last run.  It also usually ends
 *	up being the installation date.
 */

#define CONFIG_DATE "Tue Jan  1 14:34:03 MST 1991"

