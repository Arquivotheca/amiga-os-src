/* */
trace ?r
maxchars = 32888
call open infile,'vd:temp'

do forever until eof(infile)
   say eof(infile)
   rawin = readch(infile,maxchars)
   /*mask = copies('7f'x,length(rawin))*/
   rawout = bitand(rawin,,'7f'x)
   say eof(infile)
   end
