/* Audio Play */

/*
This is a supplement to the Audio.cd script.  The purpose of this script
is to run along side the audio.cd script and play a set music segment
from the center of the CD
*/

PARSE arg segTime lsnEOD
lsnEOD = strip(lsnEOD)
segTime = strip(segTime)
framesPerSec = 75

Address CD.2
OPTIONS RESULTS

say 'Playing' (segTime / framesPerSec) 'second segment at middle of disk.'
start = lsnEOD % 2 - segTime % 2
PLAYLSN start segTime

exit
