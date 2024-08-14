/* set up CyberCron for wakeup calls */

/* make sure we have rexxsupport around */
if ~show('l','rexxsupport.library') then
	addlib('rexxsupport.library',0,-30,0)

if showlist('Ports',CYBERCRON) = 0 then do
	address command 'RunWSH >nil: <nil: EXE:UsedOnlyOnce/CyberCron logfile T:CyberCronLog defstack 25000 sendmail "sendmail -f %s -R *"%s*""'
	address command 'WaitForPort CYBERCRON'
end
if showlist('Ports',CYBERCRON) = 0 then do
	say 'Unable to load CyberCron.'
	exit 10
end

address CYBERCRON

/* wake up calls */
'ADD_EVENT :NAME MonWed 15-59 7 * * 1,3 :REXX PlaySound :NOLOG'
'ADD_EVENT :NAME MonWedReset 0 8 * * 1,3 :REXX `setclip(NOSOUND)'
'ADD_EVENT :NAME TueThu 15-59 9 * * 2,4 :REXX PlaySound :NOLOG'
'ADD_EVENT :NAME TueThuReset 0 10 * * 2,4 :REXX `setclip(NOSOUND)'
'ADD_EVENT :NAME Fri 15-59 6 * * 5 :REXX PlaySound :NOLOG'
'ADD_EVENT :NAME FriReset 0 7 * * 5 :REXX `setclip(NOSOUND)'

'ADD_EVENT :NAME Sat 15-59 8 * * 6 :REXX PlaySound :NOLOG'
'ADD_EVENT :NAME SatReset 0 9 * * 6 :REXX `setclip(NOSOUND)'

'ADD_EVENT :NAME Sun 0-45 11 * * 0 :REXX PlaySound :NOLOG'
'ADD_EVENT :NAME SunReset 46 11 * * 0 :REXX `setclip(NOSOUND)'

exit 0
