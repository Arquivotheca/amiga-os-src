/* kill netrexx */
if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'socketrexx') then do
	say 'socketrexx is not running'
	exit 0
end
call addlib 'socketrexx',-1

call socket_quit()
exit 0
