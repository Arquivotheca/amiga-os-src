/* append.rexx	-- fakes a ".INCLUDE"	*/

parse arg bigfile afile .

if afile = "" then do
    call usage
    end

call addlib("rexxsupport.library", 0, -30, 0)

tmpfile = bigfile || '.tmp'
append_string = 'APPEND HERE'

/* preserve the old	*/
"copy" bigfile bigfile || '.bak'

call myopen af, afile, 'read'
call myopen tf, tmpfile, 'write'
call myopen( bf, bigfile, 'read' )

do forever
    if eof( bf ) then leave
    /* copy over everything before the APPEND string */
    buff = readln( bf )
    if index( buff, append_string ) ~= 0 then leave
    call writeln( tf, buff )
    end


    call writeln( tf,  "#########" append_string "##########" )

    /* copy over everything in the append file	*/
    do forever
    	if eof( af ) then leave
	buff = readln( af )
	call writeln( tf, buff )
	end

    /* everything is copacetic	*/
    call close( tf )
    call close( bf )
    call close( af )

    "copy" tmpfile "to" bigfile
    "delete" tmpfile

exit 0

myopen:
    parse arg fd, fname, mode
    if ~open( fd, fname, mode ) then do
	say 'can''t open file' fname ||'.'
	exit 20
	end
    return


usage:
say 'append <bigfile> <appendage>'
exit 20
