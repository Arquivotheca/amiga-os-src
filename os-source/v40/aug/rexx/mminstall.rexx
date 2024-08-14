/* mminstall.rexx - install for lmkmeta :ts=4 */

/*
 * needs to strip includes
 * should it check write access to directory?
 */

parse arg input

if ( left(address(),4) ~= 'WSH_' ) then do
	address command
	end

if input = "" then call usage

parse value input with 'strip='stripdest . , 'dir='dest . , 'files='files

/* silently bail if there are no files to install	*/
if trim(files) = "" then do
	/* say 'no files' */
	exit 0
	end

/* complain and fail if there is no destination directory	*/
if trim(dest) = "" | index( dest, 'files=') ~= 0 then do 
	say 'no destination directory given!'
	exit 20
	end

/* for statef() */
call addlib("rexxsupport.library", 0, -30, 0)

/* 
 * have destination dir and optional stripped file destination.
 * these functions will exit if they fail to find or create dirs.
 */
call validdir dest
call validdir stripdest

/*
 * process files either from 'with file' or command line
 */
if  word( files, 1 ) ~= 'with' then do
	/* command args */
	call copyflist dest, stripdest, files
	end
else do
	/* with file	*/
	wfile = word( files, 2 )
	if open( wf, wfile, 'read' ) then do
		do forever
			if eof( wf ) then exit 0
			files = readln( wf )
			call copyflist dest, stripdest, files
			end
		end
	else do
		say 'can''t open ''with file''' wfile ||'.'
		exit 20
		end
	end
exit 0

/*
 * function copyflist( dest, stripdest, file1 file2 file3 )
 *	- copies (with optional strip ) list of (single-word) files
 * Things this function must do:
 *	-silently skip files which don't exit
 *	-strip includes if stripdir is given
 */
copyflist: procedure
	parse arg ddir, stripdir, flist
	numsrc = words(flist)
	do src = 1 to numsrc
		srcname = word( flist, src )
		/* silently skip non-existent files	*/
		if  word( statef(srcname), 1) = "FILE" then

			say "	copy" srcname "to" buildname( ddir, srcname ) "nopro"
			"copy " srcname "to" buildname( ddir, srcname ) "nopro"

			if stripdir ~= "" then do
				say "	strip" srcname "to" buildname( stripdir, srcname )

				/* ZZZ: need to make sure '/' is called for	*/
				/* "stripc >nil:" srcname stripdir || '/' || srcname */
				"stripc >nil:" srcname buildname( stripdir, srcname )
				end
		end
	return

/* 
 * function buildname( dir, fname )
 * concatenates directory and file name, only
 * inserting a slash if called for (no terminating
 * colon or slash already exists)
 *
 * NOTE: this routine can be used with a null 'fname' arg,
 * to convert a directory name into a terminated directory name
 * (i.e., with slash appended, as needed).
 *
 * WARNING: If you provide a directory name terminated with
 * a slash, you had better not mean "one level up" from
 * directory named.
 */
buildname: procedure

    parse arg dir, fname .

    trailer = right( dir, 1 )

    if ( trailer = ':' | trailer = '/' ) then return dir || fname
    else return dir || '/' || fname

/*
 * function usage: 
 *	- described mminstall command syntax
 */
usage:
	say "usage: mminstall [strip=<stripdir>] dir=<dir> <file1> <file2> ..."
	say "   or: mminstall [strip=<stripdir>] dir=<dir> with <widthfile>"
	exit 20


/* function validdir( <dirname> ) 
 *	- exits if a dir exists as a file or could not be created
 */
validdir: procedure
	parse arg	ddir

	/* the null directory is considered valid, won't be used */
	if ddir = "" then return 1

	dirtype = statef(ddir)

	if word( dirtype, 1 ) == "FILE" then do
		say 'destination' ddir ' must be a directory, not a file'
		exit 20
		end

	/* might not exist	*/
	else if dirtype == "" then do
		say 'creating' ddir
		"makedir" ddir
		if word( statef( ddir ), 1 ) ~= "DIR"  then do
			say 'could not create destination dir' ddir || '.'
			exit 20
			end
		end
	return

