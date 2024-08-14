/* feedzoo.rexx - :ts=4
 *	reads file and spits out words found, a line at a time
 */

parse arg wfile

if open( wf, wfile, 'read' ) then do
	do forever
		if eof( wf ) then exit 0
		input = readln( wf )

		numwords = words(input)
		do word = 1 to numwords
			say word( input, word )
			end
		end
	end
else do
	say 'can''t open file:' wfile ||'.'
	exit 20
	end
end
