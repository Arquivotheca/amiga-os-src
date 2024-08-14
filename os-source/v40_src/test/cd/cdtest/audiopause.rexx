/*  AUDIO PAUSE */

Address CD.2
OPTIONS RESULTS

/*
This is a supplement to the Audio.cd script.  The purpose of this script
is to run along side the audio.cd script and pause the audio play at the
appropriate points in time.
*/

call delay(10)
call pawsOn
call delay(10)
call pawsOff
call delay(24)
call pawsOn       /* Step 10 */
call delay(10)
call pawsOff
call delay(8)
call pawsOn
call delay(10)
call pawsOff

say 'AudioPause.rexx is done executing'

exit

/***** FUNCTIONS ********************************************************/

pawsOn: procedure
PAUSE ON
INFO thisDisk
if thisDisk.Paused = 0 then say 'Audio play has been unpaused.'
  else say 'Audio play has been paused.'
return

pawsOff: procedure
PAUSE OFF
INFO thisDisk
if thisDisk.Paused = 0 then say 'Audio play has been unpaused.'
  else say 'Audio play has been paused.'
return

delay: procedure
arg timeindex
 /* say 'Beginning' timeindex 'second delay.' */
  call time 'R'
  do until time('E') >= timeindex
 /* if (time('E') // 2 = 0) & (time('E') > 1) then say ' Time left =' time('E') */
  end
return
