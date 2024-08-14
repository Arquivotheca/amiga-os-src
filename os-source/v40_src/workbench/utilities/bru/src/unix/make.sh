#! /bin/sh

#  FILE
#
#	make.sh    shell file to determine environment and do make
#
#  SCCS
#
#	@(#)make.sh	12.8	11 Feb 1991
#
#  SYNOPSIS
#
#	make.sh <options> <other make flags>
#
#	options include:
#
#		-copyright	compile in copyright message in usage
#		-dbug		use dbug macro debugging package
#		-huge		use huge model for xenix
#		-profile	compile with profiling enabled
#		-sdb		compile for sdb
#		-split		use split I&D model on 286 or pdp-11's
#		-small		use small model for xenix
#		-test		compile test only version (between deltas)
#
#  NOTES
#
#	We do not use "test -r" or "test -f" to test for files because
#	they seem to fail on symbolic links.  We can't use "test -h"
#	because that fails on systems without symbolic links.  We can't
#	just cat the files to /dev/null, since cat does not return nonzero
#	status for failure on pyramids.  However, cp to /dev/null seems
#	to work everywhere so far, as long as you are root.  Grrr!!!
#
#  ACKNOWLEDGEMENTS
#
#	Autoconfiguration based on a shell script "where.sh" posted to
#	usenet net.sources by Arnold Robbins of Georgia Tech.
#

DEFS=
LIBS=
split=off
huge=off
rmt=off
dbug=off
testonly=off
copyright=off
profile=off
OFLAG=-O
LINTFLAGS=
inc=/usr/include
OTHERLDFLAGS=
OTHERCFLAGS=
INCLUDE='-I../common -I../unix'
xenix=off

# For xenix, you must specify the (fixed) stack size at link time.

stack='-F 3FFF'
smsg1="If you get message 'too big' at runtime, reduce stack size by "
smsg2="changing -F parameter in $0 and relinking.  Default is"

if cp /xenix /dev/null >/dev/null 2>&1
then
	xenix=on
fi

# Parse the args first.

for i in $*
do
	case $i in
		-copyright)	copyright=on
				continue;;
		-dbug)		dbug=on
				continue;;
		-huge)		huge=on
				continue;;
		-profile)	profile=on
				OFLAG=-p
				OTHERLDFLAGS="$OTHERLDFLAGS -p"
				continue;;
		-sdb)		OFLAG=-g
				OTHERLDFLAGS="$OTHERLDFLAGS -g"
				continue;;
		-split)		split=on
				continue;;
		-small)		split=on
				stack='-F 1FFF'
				OTHERLDFLAGS="$OTHERLDFLAGS $stack"
				OTHERCFLAGS="-DCRAMPED -DHAVE_SHM=0"
				echo $smsg1
				echo $smsg2 $stack
				continue;;
		-test)		testonly=on
				continue;;
		-*)		makeflags="$makeflags $i"
				continue;;
		*=*)		makedefs="$makedefs $i"
				continue;;
		*)		targets="$targets $i"
				continue;;
	esac
done

if cp /bin/universe /dev/null >/dev/null 2>&1	# on a pyramid
then
	OPATH=$PATH
	PATH=/bin
	case `universe` in	# universe is dumb
				# looking only at argv[0]
	att)
		OTHERLDFLAGS="$OTHERLDFLAGS -n"
		;;

	ucb)
		OTHERLDFLAGS="$OTHERLDFLAGS"
		LINTFLAGS="-hbxc"
		;;

	*)	echo unknown operating system! 1>&2
		OTHERLDFLAGS="$OTHERLDFLAGS"
		;;
	esac
	PATH=$OPATH
else		# on something that is not a pyramid
	if cp /bin/sun /dev/null >/dev/null 2>&1
	then
		if /bin/sun
		then
			echo "int a = 0;" >junk.c
			if cc -S -O4 junk.c >junk.log 2>&1
			then
				OFLAG=-O4
				LINTFLAGS="-hbxc"
			fi
			rm -f junk.c junk.s junk.log
		fi
	fi
	if grep SIGTSTP $inc/signal.h > /dev/null
	then		# berkeley unix
		LINTFLAGS=-hbxc
		OTHERLDFLAGS="$OTHERLDFLAGS"
	else			# ATT unix
		if cp /zeus /dev/null >/dev/null 2>&1
		then
			OTHERLDFLAGS="$OTHERLDFLAGS -i"
		elif test "$xenix" = "on"
		then
			i80386=0
			if uname -a | grep 80386 >/dev/null 2>&1
			then
				i80386=1
			fi
			if test "$split" = "on"
			then
				# small model; 64k text and 64k data
				OTHERLDFLAGS="$OTHERLDFLAGS -i"
				OTHERCFLAGS="$OTHERCFLAGS -i"
			elif test "$huge" = "on"
			then
				# huge model
				OTHERLDFLAGS="$OTHERLDFLAGS -Mh"
				OTHERCFLAGS="$OTHERCFLAGS -Mh"
			elif test "$i80386" = "0"
			then	
				# large model; multiple text/data segs
				# set stack to appropriate value 
				OTHERLDFLAGS="$OTHERLDFLAGS -Ml $stack"
				OTHERCFLAGS="$OTHERCFLAGS -Ml"
				echo $smsg1
				echo $smsg2 $stack 
			fi
		fi
	fi
fi

if test "$dbug" = "on"
then
	DEFS="$DEFS -DDBUG"
fi

if test "$testonly" = "on"
then
	DEFS="$DEFS -DTESTONLY"
fi

if test "$copyright" = "on"
then
	DEFS="$DEFS -DCOPYRIGHT"
fi

if test "$targets" = ""
then
	targets="bru brutalk shmtest"
fi

#  Now do a make with the appropriate defines.  By defining the shell
#  variable $MAKE to be echo, you can test the actual args.  I.E., use
#  the command "MAKE=echo make.sh".

if test "$MAKE" = ""
then
	MAKE=make
fi

$MAKE DEFS="$DEFS" INCLUDE="$INCLUDE" LIBS="$LIBS" \
	OFLAG="$OFLAG" LINTFLAGS="$LINTFLAGS" OTHERCFLAGS="$OTHERCFLAGS" \
	OTHERLDFLAGS="$OTHERLDFLAGS" \
	$makedefs $makeflags $targets

exit $?

