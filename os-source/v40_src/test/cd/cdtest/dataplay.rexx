 /* Data Play */

/*
This is a supplement to the Data.cd script.  The purpose of this script
is to run along side the data.cd script and read a 512 KB data segment.
*/

Address CD.2
OPTIONS RESULTS

PARSE arg offSet
offSet = strip(offSet)

loops = 8
segLength = 512 * 1024

say 'Reading' ((segLength * loops) / 1048576) 'MB in 512 KB chunks.'

tOffSet = offSet
do i = 1 to loops
  READ tOffSet segLength
  tOffSet = tOffSet + segLength
end

/* say '*** DATA PLAY is done executing ***' */

QUIT   /* Quit from CDTEST application */
exit   /* Quit from DataPlay script */
